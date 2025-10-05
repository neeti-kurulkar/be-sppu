type ElectionStatusBadgeProps = {
  stateIndex: number;
};

export default function ElectionStatusBadge({ stateIndex }: ElectionStatusBadgeProps) {
  const getStatusConfig = () => {
    switch (stateIndex) {
      case 0:
        return {
          text: "Not Started",
          bgColor: "bg-gray-100",
          textColor: "text-gray-700",
          borderColor: "border-gray-300",
          icon: "⏸️",
        };
      case 1:
        return {
          text: "Voting Active",
          bgColor: "bg-green-100",
          textColor: "text-green-700",
          borderColor: "border-green-300",
          icon: "🗳️",
        };
      case 2:
        return {
          text: "Ended",
          bgColor: "bg-blue-100",
          textColor: "text-blue-700",
          borderColor: "border-blue-300",
          icon: "🏁",
        };
      default:
        return {
          text: "Unknown",
          bgColor: "bg-gray-100",
          textColor: "text-gray-700",
          borderColor: "border-gray-300",
          icon: "❓",
        };
    }
  };

  const config = getStatusConfig();

  return (
    <div
      className={`flex items-center gap-2 px-4 py-2 rounded-full border ${config.bgColor} ${config.textColor} ${config.borderColor}`}
    >
      <span className="text-lg">{config.icon}</span>
      <span className="font-semibold text-sm">{config.text}</span>
    </div>
  );
}