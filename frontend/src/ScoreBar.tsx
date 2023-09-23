import styles from "./ScoreBar.module.css";
export function ScoreBar({ value }: { value: number }) {
  return (
    <div className={styles.main}>
      <div className={styles.container}>
        <div className={styles.bar} style={{ width: `${value}%` }}>
          {value}
        </div>
      </div>
    </div>
  );
}
