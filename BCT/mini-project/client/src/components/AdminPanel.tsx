import { useState } from "react";
import { ethers } from "ethers";

type AdminPanelProps = {
  contract: ethers.Contract | null;
  onAction: () => Promise<void>;
  stateIndex: number;
};

export default function AdminPanel({ contract, onAction, stateIndex }: AdminPanelProps) {
  const [loading, setLoading] = useState(false);
  const [newCandidateName, setNewCandidateName] = useState("");

  const handleStartElection = async () => {
    if (!contract) return;
    setLoading(true);
    try {
      const tx = await contract.startVoting();
      await tx.wait();
      await onAction();
      alert("✅ Election started successfully!");
    } catch (err: any) {
      console.error("Start election error:", err);
      alert(`❌ Error: ${err.reason || err.message}`);
    } finally {
      setLoading(false);
    }
  };

  const handleEndElection = async () => {
    if (!contract) return;
    setLoading(true);
    try {
      const tx = await contract.endVoting();
      await tx.wait();
      await onAction();
      alert("✅ Election ended successfully!");
    } catch (err: any) {
      console.error("End election error:", err);
      alert(`❌ Error: ${err.reason || err.message}`);
    } finally {
      setLoading(false);
    }
  };

  const getStatusText = () => {
    switch (stateIndex) {
      case 0:
        return "Not Started";
      case 1:
        return "Voting in Progress";
      case 2:
        return "Ended";
      default:
        return "Unknown";
    }
  };

  return (
    <div className="space-y-4">
      {/* Current Status */}
      <div className="bg-gray-50 p-3 rounded-lg">
        <p className="text-sm font-medium text-gray-700">Current Status:</p>
        <p className="text-lg font-bold text-indigo-600">{getStatusText()}</p>
      </div>

      {/* Action Buttons */}
      <div className="space-y-2">
        {stateIndex === 0 && (
          <button
            onClick={handleStartElection}
            disabled={loading}
            className="w-full bg-green-600 hover:bg-green-700 disabled:bg-gray-400 text-white font-semibold py-2 px-4 rounded-lg transition"
          >
            {loading ? "Starting..." : "▶️ Start Election"}
          </button>
        )}

        {stateIndex === 1 && (
          <button
            onClick={handleEndElection}
            disabled={loading}
            className="w-full bg-red-600 hover:bg-red-700 disabled:bg-gray-400 text-white font-semibold py-2 px-4 rounded-lg transition"
          >
            {loading ? "Ending..." : "⏹️ End Election"}
          </button>
        )}

        {stateIndex === 2 && (
          <div className="bg-blue-50 border border-blue-200 p-3 rounded-lg text-center">
            <p className="text-sm text-blue-700 font-medium">
              🎉 Election has concluded
            </p>
          </div>
        )}
      </div>

      {/* Instructions */}
      <div className="text-xs text-gray-500 bg-gray-50 p-3 rounded border border-gray-200">
        <p className="font-semibold mb-1">Admin Controls:</p>
        <ul className="list-disc list-inside space-y-1">
          <li>Start the election when ready</li>
          <li>Monitor voting progress</li>
          <li>End election to reveal winner</li>
        </ul>
      </div>
    </div>
  );
}