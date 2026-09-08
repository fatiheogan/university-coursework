import React, { useRef, useState } from "react";
import { View, TextInput, StyleSheet } from "react-native";

type Props = { length?: number; onChange?: (code: string) => void };

export default function OtpInput({ length = 6, onChange }: Props) {
  const refs = useRef<(TextInput | null)[]>([]);
  const [vals, setVals] = useState<string[]>(Array.from({ length }, () => ""));

  const setChar = (i: number, t: string) => {
    const v = t.replace(/\D/g, "").slice(-1);
    const next = [...vals];
    next[i] = v;
    setVals(next);
    onChange?.(next.join(""));
    if (v && i < length - 1) refs.current[i + 1]?.focus();
  };

  return (
    <View style={styles.row}>
      {vals.map((val, i) => (
        <TextInput
          key={i}
          ref={(r) => { refs.current[i] = r; }}
          value={val}
          onChangeText={(t) => setChar(i, t)}
          maxLength={1}
          keyboardType="number-pad"
          style={styles.cell}
          placeholder="•"
          placeholderTextColor="rgba(255,255,255,0.6)"
        />
      ))}
    </View>
  );
}

const styles = StyleSheet.create({
  row: { flexDirection: "row", gap: 8 },
  cell: {
    width: 48, height: 56, textAlign: "center", fontSize: 22, color: "#fff",
    borderRadius: 12, backgroundColor: "rgba(255,255,255,0.2)",
    borderWidth: 1, borderColor: "rgba(255,255,255,0.2)",
  },
});