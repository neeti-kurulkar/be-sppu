import { useEffect, useState } from "react";
import { ethers } from "ethers";
import VotingABI from "./Voting.json";
import AdminPanel from "./components/AdminPanel";
import VoterDashboard from "./components/VoterDashboard";
import ElectionStatusBadge from "./components/ElectionStatusBadge";

const CONTRACT_ADDRESS = "0xea7562F7E84Bcf0f2Aa3D0a5374d6CcC283072fb";

type Candidate = {
  id: number;
  name: string;
  voteCount: number;
};

export default function App() {
  const [account, setAccount] = useState<string>("");
  const [provider, setProvider] = useState<ethers.BrowserProvider | null>(null);
  const [signer, setSigner] = useState<ethers.JsonRpcSigner | null>(null);
  const [contract, setContract] = useState<ethers.Contract | null>(null);
  const [candidates, setCandidates] = useState<Candidate[]>([]);
  const [state, setState] = useState<number>(0);
  const [isAdmin, setIsAdmin] = useState(false);
  const [hasVoted, setHasVoted] = useState(false);
  const [winner, setWinner] = useState<Candidate | null>(null);
  const [loading, setLoading] = useState(true);

  // ------------ INITIALIZATION ------------
  useEffect(() => {
    async function init() {
      if (!(window as any).ethereum) {
        alert("MetaMask not found — please install it to continue.");
        return;
      }

      try {
        const provider = new ethers.BrowserProvider((window as any).ethereum);
        setProvider(provider);
        await (window as any).ethereum.request({ method: "eth_requestAccounts" });

        const signer = await provider.getSigner();
        const addr = await signer.getAddress();

        setSigner(signer);
        setAccount(addr);

        const contract = new ethers.Contract(CONTRACT_ADDRESS, VotingABI, signer);
        setContract(contract);

        await loadAll(contract, addr);
        attachListeners(contract, addr);
      } catch (err) {
        console.error("Init error:", err);
      } finally {
        setLoading(false);
      }
    }

    init();
    return () => {
      if (contract) {
        try {
          contract.removeAllListeners();
        } catch {}
      }
    };
  }, []);

  // ------------ HELPERS ------------
  const attachListeners = (c: ethers.Contract, addr: string) => {
    // Listen for StateChanged event which fires when election starts or ends
    c.on("StateChanged", async () => {
      await loadState(c, addr);
      await loadWinnerIfEnded(c);
    });

    // Listen for Voted event
    c.on("Voted", async () => {
      await loadCandidates(c);
      const voted = await c.voters(addr);
      setHasVoted(voted);
    });
  };

  const loadAll = async (c: ethers.Contract, addr: string) => {
    await Promise.all([loadCandidates(c), loadState(c, addr), loadWinnerIfEnded(c)]);
  };

  const loadCandidates = async (c: ethers.Contract) => {
    try {
      const count = Number(await c.candidatesCount());
      const list: Candidate[] = [];
      for (let i = 1; i <= count; i++) {
        const raw = await c.getCandidate(i);
        list.push({ id: Number(raw.id), name: raw.name, voteCount: Number(raw.voteCount) });
      }
      setCandidates(list);
    } catch (err) {
      console.error("loadCandidates error:", err);
    }
  };

  const loadState = async (c: ethers.Contract, addr: string) => {
    try {
      const st = Number(await c.state());
      setState(st);
      const adminAddr = await c.admin();
      setIsAdmin(adminAddr.toLowerCase() === addr.toLowerCase());
      setHasVoted(await c.voters(addr));
    } catch (err) {
      console.error("loadState error:", err);
    }
  };

  const loadWinnerIfEnded = async (c: ethers.Contract) => {
    try {
      const st = Number(await c.state());
      if (st === 2) {
        const w = await c.getWinner();
        setWinner({ id: Number(w.id), name: w.name, voteCount: Number(w.voteCount) });
      } else setWinner(null);
    } catch (err) {
      console.error("loadWinnerIfEnded error:", err);
    }
  };

  // ------------ UI RENDER ------------
  if (loading) {
    return (
      <div className="flex items-center justify-center min-h-screen bg-gray-50">
        <div className="text-gray-600">🔄 Connecting to MetaMask...</div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-gray-50 flex flex-col items-center py-10 px-4">
      <div className="w-full max-w-5xl">
        <header className="flex flex-col sm:flex-row sm:items-center sm:justify-between mb-10 gap-3">
          <div>
            <h1 className="text-3xl font-bold text-indigo-700">🗳️ Decentralized Voting</h1>
            <p className="text-sm text-gray-600">
              Connected wallet: <span className="font-mono text-indigo-600">{account}</span>
            </p>
          </div>
          <ElectionStatusBadge stateIndex={state} />
        </header>

        <main className="grid grid-cols-1 md:grid-cols-3 gap-8">
          {/* Candidate / Voting Panel */}
          <section className="md:col-span-2 bg-white rounded-xl shadow-md p-6 border border-gray-100">
            <h2 className="text-lg font-semibold mb-4">Candidates</h2>

            <VoterDashboard
              contract={contract}
              candidates={candidates}
              hasVoted={hasVoted}
              stateIndex={state}
              onVoted={async () => {
                if (contract && account) {
                  await loadCandidates(contract);
                  await loadState(contract, account);
                }
              }}
            />

            {hasVoted && (
              <div className="mt-4 text-green-600 bg-green-50 border border-green-200 px-3 py-2 rounded-lg">
                ✅ You have successfully voted!
              </div>
            )}
          </section>

          {/* Admin Panel */}
          <aside className="bg-white rounded-xl shadow-md p-6 border border-gray-100">
            <h3 className="text-lg font-semibold mb-4">Admin Panel</h3>
            {isAdmin ? (
              <AdminPanel
                contract={contract}
                onAction={async () => {
                  if (contract && account) {
                    await loadAll(contract, account);
                  }
                }}
                stateIndex={state}
              />
            ) : (
              <p className="text-sm text-gray-600">
                You are <span className="font-semibold">not the admin</span>. Only the admin can manage the election.
              </p>
            )}

            {/* Summary */}
            <div className="mt-8 border-t pt-4">
              <h4 className="font-semibold mb-2">Election Summary</h4>
              <p className="text-sm text-gray-600">Total candidates: {candidates.length}</p>
              <p className="text-sm text-gray-600">
                Total votes: {candidates.reduce((a, b) => a + b.voteCount, 0)}
              </p>
              {winner && (
                <div className="mt-4 p-3 rounded-lg bg-green-50 border border-green-200">
                  🏆 <strong>Winner:</strong> {winner.name} ({winner.voteCount} votes)
                </div>
              )}
            </div>
          </aside>
        </main>
      </div>
    </div>
  );
}