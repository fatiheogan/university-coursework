import React, { useState } from "react";
import { Text, View, Pressable } from "react-native";
import Gradient from "@/components/Gradient";
import BackButton from "@/components/BackButton";
import Button from "@/components/Button";
import OtpInput from "@/components/OtpInput";


export default function Activate() {
    const [code, setCode] = useState("");


    return (
        <Gradient>
            <BackButton />
            <Text style={{ color: "white", fontSize: 32, fontWeight: "800", marginBottom: 16 }}>Aktivasyon</Text>
            <View style={{ gap: 18 }}>
                <OtpInput length={6} onChange={setCode} />
                <Text style={{ color: "rgba(255,255,255,0.9)" }}>E‑postana gelen 6 haneli doğrulama kodunu gir</Text>
                <Button disabled={code.length < 6}>Doğrula</Button>
                <Pressable>
                    <Text style={{ color: "rgba(255,255,255,0.9)", textDecorationLine: "underline" }}>Kodu yeniden gönder</Text>
                </Pressable>
            </View>
            <Text style={{ position: "absolute", bottom: 10, alignSelf: "center", color: "rgba(255,255,255,0.7)" }}>indisakademi</Text>
        </Gradient>
    );
}