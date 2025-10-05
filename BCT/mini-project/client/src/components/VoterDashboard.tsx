import { useState } from "react";
import { ethers } from "ethers";

type Candidate = {
  id: number;
  name: string;
  voteCount: number;
};

type VoterDashboardProps = {
  contract: ethers.Contract | null;
  candidates: Candidate[];
  hasVoted: boolean;
  stateIndex: number;
  onVoted: () => Promise<void>;
};

export default function VoterDashboard({
  contract,
  candidates,
  hasVoted,
  stateIndex,
  onVoted,
}: VoterDashboardProps) {
  const [loading, setLoading] = useState(false);
  const [votingFor, setVotingFor] = useState<number | null>(null);

  const handleVote = async (candidateId: number) => {
    if (!contract) return;
    setLoading(true);
    setVotingFor(candidateId);
    try {
      const tx = await contract.vote(candidateId);
      await tx.wait();
      await onVoted();
      alert("✅ Vote submitted successfully!");
    } catch (err: any) {
      console.error("Vote error:", err);
      alert(`❌ Error: ${err.reason || err.message}`);
    } finally {
      setLoading(false);
      setVotingFor(null);
    }
  };

  // Calculate total votes for percentage
  const totalVotes = candidates.reduce((sum, c) => sum + c.voteCount, 0);

  if (candidates.length === 0) {
    return (
      <div className="text-center py-8 text-gray-500">
        No candidates available yet.
      </div>
    );
  }

  if (stateIndex === 0) {
    return (
      <div className="text-center py-8 bg-yellow-50 border border-yellow-200 rounded-lg">
        <p className="text-yellow-700 font-medium">
          ⏳ Election has not started yet. Please wait for the admin to start the election.
        </p>
      </div>
    );
  }

  if (stateIndex === 2) {
    return (
      <div className="space-y-3">
        <div className="text-center py-4 bg-blue-50 border border-blue-200 rounded-lg mb-4">
          <p className="text-blue-700 font-medium">
            🎉 Election has ended. Results are final!
          </p>
        </div>
        {candidates.map((candidate) => {
          const percentage = totalVotes > 0 ? (candidate.voteCount / totalVotes) * 100 : 0;
          return (
            <div
              key={candidate.id}
              className="border border-gray-200 rounded-lg p-4 bg-gray-50"
            >
              <div className="flex justify-between items-center mb-2">
                <span className="font-semibold text-gray-800">{candidate.name}</span>
                <span className="text-sm font-medium text-gray-600">
                  {candidate.voteCount} votes ({percentage.toFixed(1)}%)
                </span>
              </div>
              <div className="w-full bg-gray-200 rounded-full h-2.5">
                <div
                  className="bg-indigo-600 h-2.5 rounded-full transition-all duration-500"
                  style={{ width: `${percentage}%` }}
                ></div>
              </div>
            </div>
          );
        })}
      </div>
    );
  }

  // Voting is active (stateIndex === 1)
  return (
    <div className="space-y-3">
      {candidates.map((candidate) => {
        const percentage = totalVotes > 0 ? (candidate.voteCount / totalVotes) * 100 : 0;
        const isVoting = votingFor === candidate.id;
        
        return (
          <div
            key={candidate.id}
            className="border border-gray-200 rounded-lg p-4 hover:border-indigo-300 transition"
          >
            <div className="flex justify-between items-center mb-2">
              <div className="flex-1">
                <span className="font-semibold text-gray-800">{candidate.name}</span>
                <div className="text-sm text-gray-600 mt-1">
                  {candidate.voteCount} votes ({percentage.toFixed(1)}%)
                </div>
              </div>
              <button
                onClick={() => handleVote(candidate.id)}
                disabled={loading || hasVoted}
                className={`px-4 py-2 rounded-lg font-semibold transition ${
                  hasVoted
                    ? "bg-gray-300 text-gray-500 cursor-not-allowed"
                    : "bg-indigo-600 hover:bg-indigo-700 text-white"
                } ${isVoting ? "opacity-50" : ""}`}
              >
                {isVoting ? "Voting..." : hasVoted ? "Voted" : "Vote"}
              </button>
            </div>
            <div className="w-full bg-gray-200 rounded-full h-2">
              <div
                className="bg-indigo-600 h-2 rounded-full transition-all duration-500"
                style={{ width: `${percentage}%` }}
              ></div>
            </div>
          </div>
        );
      })}
    </div>
  );
}