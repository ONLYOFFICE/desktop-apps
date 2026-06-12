export function htmlDecode(str) {
  const ta = document.createElement("textarea");
  ta.innerHTML = str;
  return ta.value;
}
