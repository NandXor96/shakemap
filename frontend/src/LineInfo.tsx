import { MapGeoJSONFeature } from "react-map-gl/maplibre";

type Props = { line: MapGeoJSONFeature };
export function LineInfo({ line }: Props) {
  const points = JSON.parse(line.properties.point_ids);
  return (
    <div>
      <div>Wert: {line.properties.value}</div>
      <div>
        {points.length} {points.length < 2 ? "Meldung" : "Meldungen"}
      </div>
    </div>
  );
}
