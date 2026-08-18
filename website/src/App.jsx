import React, { useState } from 'react';
import { translations } from './translations';
import Navbar from './components/Navbar';
import Hero from './components/Hero';
import AudioComparator from './components/AudioComparator';
import AppSimulator from './components/AppSimulator';
import Features from './components/Features';
import Benchmarks from './components/Benchmarks';
import Compatibility from './components/Compatibility';
import HowItWorks from './components/HowItWorks';
import PricingSection from './components/PricingSection';
import FaqSection from './components/FaqSection';
import CtaBanner from './components/CtaBanner';
import Footer from './components/Footer';
import DownloadModal from './components/DownloadModal';

export default function App() {
  const [lang, setLang] = useState('es');
  const [isDownloadOpen, setIsDownloadOpen] = useState(false);

  const t = translations[lang] || translations.es;

  const handleOpenDownload = () => {
    setIsDownloadOpen(true);
  };

  return (
    <div className="min-h-screen bg-[#06080C] text-slate-100 selection:bg-brand-cyan selection:text-black">
      {/* Top Navbar */}
      <Navbar
        lang={lang}
        setLang={setLang}
        t={t}
        onOpenDownload={handleOpenDownload}
      />

      {/* Main Content Sections */}
      <main>
        <Hero
          t={t}
          onOpenDownload={handleOpenDownload}
        />

        <AudioComparator
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
          onOpenDownload={handleOpenDownload}
        />

        <FaqSection
          t={t}
        />

        <CtaBanner
          t={t}
          onOpenDownload={handleOpenDownload}
        />
      </main>

      {/* Footer */}
      <Footer
        t={t}
      />

      {/* Download Modal Popup */}
      <DownloadModal
        isOpen={isDownloadOpen}
        onClose={() => setIsDownloadOpen(false)}
        t={t}
      />
    </div>
  );
}
