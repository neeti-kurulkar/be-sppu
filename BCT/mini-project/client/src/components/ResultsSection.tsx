export default function ResultsSection({ candidates }: { candidates: { id: number; name: string; voteCount: number }[] }) {
  if (candidates.length === 0) return null;
  const total = candidates.reduce((a, b) => a + b.voteCount, 0);

  return (
    <div className="mt-6 bg-white rounded p-4 shadow">
      <h4 className="font-semibold mb-3">Results</h4>
      <div className="space-y-2">
        {candidates.map((c) => {
          const pct = total === 0 ? 0 : Math.round((c.voteCount / total) * 100);
          return (
            <div key={c.id}>
              <div className="flex justify-between">
                <div>{c.name}</div>
                <div>{c.voteCount} ({pct}%)</div>
              </div>
              <div className="h-2 bg-gray-200 rounded mt-2">
                <div style={{ width: `${pct}%` }} className="h-2 bg-indigo-600 rounded" />
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
}