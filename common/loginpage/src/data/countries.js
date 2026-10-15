export const TEMPLATES_COUNTRY_ARRAY = [
  { value: "en", label: "United States", sublabel: "" },
  { value: "zh", label: "中國", sublabel: "China" },
  { value: "es", label: "España", sublabel: "Spain" },
  { value: "de", label: "Duitsland", sublabel: "Germany" },
  { value: "pt", label: "Portugal", sublabel: "Portugal" },
  { value: "fr", label: "France", sublabel: "France" },
  { value: "it", label: "Italia", sublabel: "Italy" },
  { value: "ja", label: "日本", sublabel: "Japan" },
  { value: "ar", label: "المملكة العربية السعودية", sublabel: "Saudi Arabia" },
];

export const TemplatesCountrySet = new Set(TEMPLATES_COUNTRY_ARRAY.map((o) => o.value));
