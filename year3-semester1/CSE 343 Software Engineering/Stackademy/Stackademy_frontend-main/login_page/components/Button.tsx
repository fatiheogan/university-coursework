import React from "react";
import { Pressable, Text, StyleSheet, ViewStyle } from "react-native";


type Props = React.PropsWithChildren<{ variant?: "primary" | "secondary"; style?: ViewStyle; onPress?: () => void; disabled?: boolean }>;


export default function Button({ children, variant = "primary", style, ...props }: Props) {
    return (
        <Pressable
            style={({ pressed }) => [
                styles.base,
                variant === "primary" ? styles.primary : styles.secondary,
                pressed && { opacity: 0.9 },
                style,
            ]}
            {...props}
        >
            <Text style={variant === "primary" ? styles.textPrimary : styles.textSecondary}>{children}</Text>
        </Pressable>
    );
}
const styles = StyleSheet.create({
    base: { width: "100%", borderRadius: 14, paddingVertical: 14, alignItems: "center" },
    primary: { backgroundColor: "#0B2B7E" },
    secondary: { borderWidth: 1, borderColor: "rgba(255,255,255,0.2)" },
    textPrimary: { color: "#fff", fontWeight: "700" },
    textSecondary: { color: "#fff", fontWeight: "700" },
});