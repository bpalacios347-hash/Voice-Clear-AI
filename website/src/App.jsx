import React, { useState } from 'react';
import { translations } from './translations';
import Navbar from './components/Navbar';
import Hero from './components/Hero';
import AppSimulator from './components/AppSimulator';
import Features from './components/Features';
import Benchmarks from './components/Benchmarks';
import Compatibility from './components/Compatibility';
import HowItWorks from './components/HowItWorks';
import PricingSection from './components/PricingSection';
import FaqSection from './components/FaqSection';
import CtaBanner from './components/CtaBanner';
import Footer from './components/Footer';

export default function App() {
  const [lang, setLang] = useState('es');

  const t = translations[lang] || translations.es;

  return (
    <div className="min-h-screen bg-[#06080C] text-slate-100 selection:bg-brand-cyan selection:text-black">
      {/* Top Navbar */}
      <Navbar
        lang={lang}
        setLang={setLang}
        t={t}
      />

      {/* Main Content Sections */}
      <main>
        <Hero
          t={t}
        />

        <AppSimulator
          t={t}
        />

        <Features
          t={t}
        />

        <Benchmarks
          t={t}
        />

        <Compatibility
          t={t}
        />

        <HowItWorks
          t={t}
        />

        <PricingSection
          t={t}
        />

        <FaqSection
          t={t}
        />

        <CtaBanner
          t={t}
        />
      </main>

      {/* Footer */}
      <Footer
        t={t}
      />
    </div>
  );
}
