import type { Metadata } from "next";
import { Geist_Mono } from "next/font/google";
import "./globals.css";
import { MidiProvider } from "./context/MidiContext";
import { ThemeProvider } from "next-themes";
import ThemeToggle from "./components/ThemeToggle";

const geistMono = Geist_Mono({
  variable: "--font-geist-mono",
  subsets: ["latin"],
});

export const metadata: Metadata = {
  title: "DSL Compiler",
  description: "Web IDE for the DSL music compiler",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en" className={`${geistMono.variable} h-full`} suppressHydrationWarning>
      <body className="h-full flex flex-col bg-background text-foreground antialiased">
        <ThemeProvider attribute="data-theme" defaultTheme="dark">
          <header className="shrink-0 flex items-center justify-between px-4 h-12 border-b border-border bg-toolbar">
            <span className="font-mono text-sm font-semibold tracking-widest uppercase">
              DSL Compiler
            </span>
            <ThemeToggle />
          </header>
          <MidiProvider>
            <main className="flex flex-1 min-h-0">{children}</main>
          </MidiProvider>
        </ThemeProvider>
      </body>
    </html>
  );
}
