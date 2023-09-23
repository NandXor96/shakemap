import { MapGeoJSONFeature } from "react-map-gl/maplibre";
import { ScoreBar } from "./ScoreBar";
import { ScoreText } from "./ScoreText";

import styles from "./LineInfo.module.css";

type Props = { line: MapGeoJSONFeature };
export function LineInfo({ line }: Props) {
  const points = JSON.parse(line.properties.point_ids);
  const images = JSON.parse(line.properties.image_urls);
  const imageNodes = images.map((url: string) => <img key={url} src={url} />);
  return (
    <div className={styles.info}>
      <p>
        Zustand: <ScoreText value={line.properties.value} />
        <ScoreBar value={line.properties.value} />
      </p>

      <div>
        {points.length} {points.length < 2 ? "Meldung" : "Meldungen"}
      </div>
      <br />
      <div className={styles.images}>{imageNodes}</div>
    </div>
  );
}
