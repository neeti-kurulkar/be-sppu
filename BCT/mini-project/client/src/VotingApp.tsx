import { useEffect, useState } from "react";
import { ethers } from "ethers";
import VotingABI from "./Voting.json";

const CONTRACT_ADDRESS = "0x802BB580B0Fe27ea76bDD04a8DfeCE81e56F2195";

interface Candidate {
  id: number;
  name: string;
  voteCount: number;
}

export default function VotingApp() {
  const [account, setAccount] = useState<string>("");
  const [candidates, setCandidates] = useState<Candidate[]>([]);
  const [voting, setVoting] = useState<ethers.Contract | null>(null);
  const [hasVoted, setHasVoted] = useState(false);

  // Initialize connection & fetch candidates
  useEffect(() => {
    async function init() {
      if (!(window as any).ethereum) {
        alert("MetaMask not detected!");
        return;
      }

      await (window as any).ethereum.request({ method: "eth_requestAccounts" });
      const provider = new ethers.BrowserProvider((window as any).ethereum);
      const signer = await provider.getSigner();
      const addr = await signer.getAddress();
      setAccount(addr);

      const contract = new ethers.Contract(CONTRACT_ADDRESS, VotingABI, signer);
      setVoting(contract);

      // Load candidates
      await loadCandidates(contract);

      // Check if user has voted
      const voted = await contract.voters(addr);
      setHasVoted(voted);

      // Listen for new votes
      contract.on("Voted", async () => {
        await loadCandidates(contract);
        const votedNow = await contract.voters(addr);
        setHasVoted(votedNow);
      });
    }

    init();
  }, []);

  // Fetch candidates helper
  const loadCandidates = async (contract: ethers.Contract) => {
    const count = Number(await contract.candidatesCount());
    const list: Candidate[] = [];
    for (let i = 1; i <= count; i++) {
      const c = await contract.getCandidate(i);
      list.push({
        id: Number(c.id),
        name: c.name,
        voteCount: Number(c.voteCount),
      });
    }
    setCandidates(list);
  };

  // Vote handler
  const vote = async (id: number) => {
    if (!voting) return;
    try {
      const tx = await voting.vote(id);
      await tx.wait();
      alert("Vote successful!");
    } catch (err: any) {
      alert(err.message);
    }
  };

  // Compute winner
  const winner = candidates.reduce((prev, curr) => (curr.voteCount > prev.voteCount ? curr : prev), { id: 0, name: "", voteCount: 0 });

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 to-purple-50 flex flex-col items-center py-10">
      <h1 className="text-4xl font-bold text-gray-800 mb-4">🗳️ Voting dApp</h1>
      <p className="text-gray-600 mb-8">Connected account: <span className="font-mono">{account}</span></p>

      <div className="w-full max-w-md space-y-4">
        {candidates.map((c) => (
          <div key={c.id} className="flex justify-between items-center bg-white shadow-md rounded-lg p-4 hover:shadow-xl transition-shadow">
            <div>
              <p className="text-lg font-semibold text-gray-800">{c.name}</p>
              <p className="text-gray-500">Votes: {c.voteCount}</p>
            </div>
            <button
              onClick={() => vote(c.id)}
              disabled={hasVoted}
              className={`px-4 py-2 rounded-lg transition-colors ${
                hasVoted ? "bg-gray-400 cursor-not-allowed" : "bg-purple-600 text-white hover:bg-purple-700"
              }`}
            >
              {hasVoted ? "Already Voted" : "Vote"}
            </button>
          </div>
        ))}
      </div>

      {candidates.length > 0 && (
        <p className="mt-6 font-bold text-lg">
          Current Leader: {winner.name} ({winner.voteCount} votes)
        </p>
      )}
    </div>
  );
}