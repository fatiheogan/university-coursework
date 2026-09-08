import React from "react";
import { Text, View } from "react-native";
import { Link } from "expo-router";
import Gradient from "@/components/Gradient";
import Button from "@/components/Button";


export default function Welcome() {
    return (
        <Gradient>
            <View style={{ flex: 1, justifyContent: "center" }}>
                <View style={{ alignItems: "center" }}>
                    <Text style={{ color: "white", fontSize: 40, fontWeight: "700" }}>
                        indis<Text style={{ fontWeight: "300" }}>akademi</Text>
                    </Text>
                </View>
                <View style={{ marginTop: 36, gap: 12 }}>
                    <Link href="/login" asChild>
                        <Button>Giriş yap</Button>
                    </Link>
                    <Link href="/register" asChild>
                        <Button variant="secondary">Kayıt ol</Button>
                    </Link>
                </View>
            </View>
            <Text style={{ position: "absolute", bottom: 10, alignSelf: "center", color: "rgba(255,255,255,0.7)" }}>indisakademi</Text>
        </Gradient>
    );
}