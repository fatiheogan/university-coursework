import React from "react";
import { TextInput, StyleSheet, TextInputProps } from "react-native";


export default function Input(props: TextInputProps) {
    return (
        <TextInput
            placeholderTextColor="rgba(255,255,255,0.7)"
            style={styles.input}
            {...props}
        />
    );
}
const styles = StyleSheet.create({
    input: {
        width: "100%",
        borderRadius: 14,
        paddingHorizontal: 14,
        paddingVertical: 12,
        backgroundColor: "rgba(255,255,255,0.2)",
        color: "#fff",
        borderWidth: 1,
        borderColor: "rgba(255,255,255,0.2)",
    },
});