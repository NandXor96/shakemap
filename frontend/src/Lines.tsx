import { Layer, Source, LineLayer } from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";

import type { FeatureCollection } from "geojson";

const lineLayer: LineLayer = {
  id: "line",
  source: "line",
  type: "line",
  paint: {
    "line-color": [
      "interpolate",
      ["linear"],
      ["get", "value"],
      1,
      "orange",
      5,
      "red",
    ],

    "line-width": ["interpolate", ["linear"], ["zoom"], 12, 1, 20, 5],
  },
  layout: { "line-cap": "round" },
};

export function Lines({
  id,
  data,
}: {
  id?: string;
  data?: FeatureCollection;
  withHeatmap?: boolean;
}) {
  return (
    <Source id="line" type="geojson" data={data}>
      <Layer {...lineLayer} id={id} beforeId="label_water" />
    </Source>
  );
}
