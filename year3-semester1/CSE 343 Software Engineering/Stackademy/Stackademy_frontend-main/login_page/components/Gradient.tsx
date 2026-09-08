import React from "react";
import { LinearGradient } from "expo-linear-gradient";
import { StyleSheet } from "react-native";


export default function Gradient({ children }: { children: React.ReactNode }) {
    return (
        <LinearGradient colors={["#0B5BD3", "#0A3FA5"]} style={styles.wrap}>
            {children}
        </LinearGradient>
    );
}
const styles = StyleSheet.create({
    wrap: { flex: 1, paddingHorizontal: 20, paddingTop: 24, paddingBottom: 16 },
});