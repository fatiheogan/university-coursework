import React from "react";
import { Text, View, Pressable } from "react-native";
import { Link } from "expo-router";
import Gradient from "@/components/Gradient";
import BackButton from "@/components/BackButton";
import Input from "@/components/Input";
import Button from "@/components/Button";


export default function Login() {
    return (
        <Gradient>
            <BackButton />
            <Text style={{ color: "white", fontSize: 32, fontWeight: "800", marginBottom: 16 }}>Giriş yap</Text>
            <View style={{ gap: 12 }}>
                <Input placeholder="Kullanıcı adı veya e‑posta" />
                <Input placeholder="Şifre" secureTextEntry />
                <Pressable style={{ alignSelf: "flex-end" }}>
                    <Text style={{ color: "rgba(255,255,255,0.9)" }}>Şifremi unuttum</Text>
                </Pressable>
                <Button>Giriş yap</Button>
                <Text style={{ alignSelf: "center", color: "rgba(255,255,255,0.9)" }}>
                  Hesabın yok mu? <Link href="/register" style={{ textDecorationLine: "underline", color: "#fff" }}>Hemen kaydol!</Link>
                </Text>
            </View>
            <Text style={{ position: "absolute", bottom: 10, alignSelf: "center", color: "rgba(255,255,255,0.7)" }}>indisakademi</Text>
        </Gradient>
    );
}