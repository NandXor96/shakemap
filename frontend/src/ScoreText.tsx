export function ScoreText({ value }: { value: number }) {
  return <span>{getText(value)}</span>;
}

function getText(value: number): string {
  value = Math.round(value);
  if (value == 1) return "Sehr gut";
  if (value == 2) return "Gut";
  if (value == 3) return "Mittel";
  if (value == 4) return "Unzureichend";
  if (value == 5) return "Unbefahrbar";
  return "Unbekannt";
}
