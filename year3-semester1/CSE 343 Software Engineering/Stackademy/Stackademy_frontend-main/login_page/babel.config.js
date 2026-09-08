module.exports = function (api) {
  api.cache(true);
  return {
    presets: ["babel-preset-expo"],     // ✅ router desteği preset'in içinde
    plugins: [
      "react-native-reanimated/plugin", // ✅ EN SONDA
    ],
  };
};