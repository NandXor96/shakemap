import styles from "./ScoreBar.module.css";
export function ScoreBar({ value }: { value: number }) {
  return (
    <div className={styles.main}>
      <div className={styles.container}>
        <div
          className={styles.bar}
          style={{ width: `${((6 - value) / 5) * 100}%` }}
        >
          {Math.round(value * 10) / 10}
        </div>
      </div>
    </div>
  );
}
