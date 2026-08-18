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
import Testimonials from './components/Testimonials';
import FaqSection from './components/FaqSection';
import CtaBanner from './components/CtaBanner';
import Footer from './components/Footer';
import DownloadModal from './components/DownloadModal';
import CheckoutModal from './components/CheckoutModal';

export default function App() {
  const [lang, setLang] = useState('es');
  const [isDownloadOpen, setIsDownloadOpen] = useState(false);
  const [isCheckoutOpen, setIsCheckoutOpen] = useState(false);
  const [selectedPlan, setSelectedPlan] = useState('pro');

  const t = translations[lang] || translations.es;

  const handleOpenDownload = () => {
    setIsDownloadOpen(true);
  };

  const handleOpenCheckout = (planId = 'pro') => {
    setSelectedPlan(planId);
    setIsCheckoutOpen(true);
  };

  return (
    <div className="min-h-screen bg-[#06080C] text-slate-100 selection:bg-brand-cyan selection:text-black">
      {/* Top Navbar */}
      <Navbar
        lang={lang}
        setLang={setLang}
        t={t}
        onOpenDownload={handleOpenDownload}
        onOpenCheckout={handleOpenCheckout}
      />

      {/* Main Content Sections */}
      <main>
        <Hero
          t={t}
          onOpenDownload={handleOpenDownload}
          onOpenCheckout={handleOpenCheckout}
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
          onOpenCheckout={handleOpenCheckout}
        />

        <Testimonials
          t={t}
        />

        <FaqSection
          t={t}
        />

        <CtaBanner
          t={t}
          onOpenDownload={handleOpenDownload}
          onOpenCheckout={handleOpenCheckout}
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

      {/* Checkout Purchase Modal Popup */}
      <CheckoutModal
        isOpen={isCheckoutOpen}
        onClose={() => setIsCheckoutOpen(false)}
        selectedPlanId={selectedPlan}
        t={t}
      />
    </div>
  );
}
