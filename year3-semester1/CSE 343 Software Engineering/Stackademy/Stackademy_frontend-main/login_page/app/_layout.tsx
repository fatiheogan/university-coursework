import { Stack } from "expo-router";
import { SafeAreaView } from "react-native-safe-area-context";
import { StatusBar } from "expo-status-bar";
import React from "react";

export default function RootLayout() {
  return (
    <SafeAreaView style={{ flex: 1 }}>
     <StatusBar style="light" />
     <Stack screenOptions={{ headerShown: false }} />
    </SafeAreaView>
  );
}