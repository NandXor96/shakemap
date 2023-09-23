import { Layer, Source, LineLayer } from "react-map-gl/maplibre";
import type { FeatureCollection } from "geojson";
import { THRESHOLD } from "./const";

const lineLayer: LineLayer = {
  id: "line",
  source: "lane",
  type: "line",
  paint: {
    "line-color": "#FCDD2E",
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
  layout: {
    "line-cap": "round",
  },
};
type Props = { data?: FeatureCollection };
export function Lanes({ data }: Props) {
  return (
    <Source id="lane" type="geojson" data={data}>
      <Layer {...lineLayer} beforeId="label_water" />
    </Source>
  );
}
