import React from "react";
import { Text, View } from "react-native";
import { useRouter, Link } from "expo-router";
import Gradient from "@/components/Gradient";
import BackButton from "@/components/BackButton";
import Input from "@/components/Input";
import Button from "@/components/Button";


export default function Register() {
    const router = useRouter();
    const go = () => router.push("/activate");


    return (
        <Gradient>
            <BackButton />
            <Text style={{ color: "white", fontSize: 32, fontWeight: "800", marginBottom: 16 }}>Kayıt ol</Text>
            <View style={{ gap: 12 }}>
                <Input placeholder="İsim-Soyisim" />
                <Input placeholder="Kullanıcı adı" />
                <Input placeholder="E-posta" keyboardType="email-address" />
                <Input placeholder="Telefon numarası" keyboardType="phone-pad" />
                <Input placeholder="Şifre" secureTextEntry />
                <Button onPress={go}>Kayıt ol</Button>
                <Text style={{ alignSelf: "center", color: "rgba(255,255,255,0.9)" }}>
                  Zaten üye misin? <Link href="/login" style={{ textDecorationLine: "underline", color: "#fff" }}>Giriş yap!</Link>
                </Text>
            </View>
            <Text style={{ position: "absolute", bottom: 10, alignSelf: "center", color: "rgba(255,255,255,0.7)" }}>indisakademi</Text>
        </Gradient>
    );
}