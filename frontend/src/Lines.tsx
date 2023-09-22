import { Layer, Source, LineLayer } from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";

import type { FeatureCollection } from "geojson";

import { THRESHOLD } from "./const";

const lineLayer: LineLayer = {
  id: "line",
  source: "line",
  type: "line",
  paint: {
    "line-color": [
      "interpolate",
      ["linear"],
      ["get", "value"],
      THRESHOLD,
      "orange",
      100,
      "red",
    ],

    "line-opacity": [
      "interpolate",
      ["linear"],
      ["get", "value"],
      0,
      0,
      THRESHOLD,
      0,
      THRESHOLD + 0.1,
      1,
    ],

    "line-width": ["interpolate", ["linear"], ["zoom"], 12, 1, 20, 5],
  },
  layout: { "line-cap": "round" },
};

export function Lines({
  data,
}: {
  data?: FeatureCollection;
  withHeatmap?: boolean;
}) {
  return (
    <Source id="line" type="geojson" data={data}>
      <Layer {...lineLayer} />
    </Source>
  );
}
