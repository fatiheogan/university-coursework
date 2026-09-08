import React, { useEffect } from "react";
import { Text, View, Pressable } from "react-native";
import { useRouter } from "expo-router";
import Gradient from "@/components/Gradient";


export default function Splash() {
  const router = useRouter();
  useEffect(() => {
    const t = setTimeout(() => router.replace("/welcome"), 1200);
    return () => clearTimeout(t);
  }, [router]);


return (
  <Gradient>
    <View style={{ flex: 1, alignItems: "center", justifyContent: "center" }}>
      <Pressable onPress={() => router.replace("/welcome")}>
        <View style={{ alignItems: "center" }}>
          <Text style={{ color: "white", fontSize: 40, fontWeight: "700" }}>
            indis<Text style={{ fontWeight: "300" }}>akademi</Text>
          </Text>
          <Text style={{ marginTop: 6, color: "rgba(255,255,255,0.8)" }}>Devam etmek için dokunun</Text>
        </View>
      </Pressable>
    </View>
    <Text style={{ position: "absolute", bottom: 10, alignSelf: "center", color: "rgba(255,255,255,0.7)" }}>indisakademi</Text>
  </Gradient>
  );
}