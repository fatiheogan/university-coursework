import React from "react";
import { Pressable, Text } from "react-native";
import { useRouter } from "expo-router";


export default function BackButton() {
    const router = useRouter();
    return (
        <Pressable onPress={() => router.back()} style={{ marginBottom: 12 }}>
            <Text style={{ color: "white", fontSize: 18 }}>←</Text>
        </Pressable>
    );
}