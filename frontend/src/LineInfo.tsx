import { MapGeoJSONFeature } from "react-map-gl/maplibre";
import { ScoreBar } from "./ScoreBar";
import { ScoreText } from "./ScoreText";

import styles from "./LineInfo.module.css";

type Props = { line: MapGeoJSONFeature };
export function LineInfo({ line }: Props) {
  const points = JSON.parse(line.properties.point_ids);
  return (
    <div className={styles.info}>
      <p>
        Zustand: <ScoreText value={line.properties.value} />
        <ScoreBar value={line.properties.value} />
      </p>

      <div>
        {points.length} {points.length < 2 ? "Meldung" : "Meldungen"}
      </div>
    </div>
  );
}
