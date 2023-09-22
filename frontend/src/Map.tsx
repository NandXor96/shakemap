import { useRef, useEffect } from "react";
import maplibregl from "maplibre-gl";
import "maplibre-gl/dist/maplibre-gl.css";

import styles from "./Map.module.css";

export function Map() {
  const mapContainerRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!mapContainerRef.current) return;

    const map = new maplibregl.Map({
      container: mapContainerRef.current,
      style: "https://maps.moritz.tk/style.json",
      center: [7.6261, 51.9607],
      zoom: 14,
    });

    return () => {
      map.remove();
    };
  }, []);

  return (
    <div className={styles.wrapper}>
      <div ref={mapContainerRef} className={styles.map} />
    </div>
  );
}
