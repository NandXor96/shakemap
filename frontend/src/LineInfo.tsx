import { MapGeoJSONFeature } from "react-map-gl/maplibre";
import { ScoreBar } from "./ScoreBar";

type Props = { line: MapGeoJSONFeature };
export function LineInfo({ line }: Props) {
  const points = JSON.parse(line.properties.point_ids);
  return (
    <div>
      <p>
        Zustand
        <ScoreBar value={line.properties.value} />
      </p>

      <div>
        {points.length} {points.length < 2 ? "Meldung" : "Meldungen"}
      </div>
    </div>
  );
}
