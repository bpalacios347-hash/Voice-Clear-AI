import React, { useState, useEffect } from 'react';
import { Globe, Menu, X, MessageCircle } from 'lucide-react';

export default function Navbar({ lang, setLang, t }) {
  const [scrolled, setScrolled] = useState(false);
  const [mobileMenuOpen, setMobileMenuOpen] = useState(false);

  useEffect(() => {
    const handleScroll = () => {
      setScrolled(window.scrollY > 20);
    };
    window.addEventListener('scroll', handleScroll);
    return () => window.removeEventListener('scroll', handleScroll);
  }, []);

  const openWhatsApp = (msg) => {
    const text = msg || "Hola, quiero información sobre Voice Clear AI ($6.99 USD).";
    window.open(`https://wa.me/50587414791?text=${encodeURIComponent(text)}`, '_blank');
  };

  return (
    <header className={`fixed top-0 left-0 right-0 z-50 transition-all duration-300 ${
      scrolled 
        ? 'bg-brand-darker/95 backdrop-blur-md border-b border-brand-border py-3 shadow-2xl shadow-black/50' 
        : 'bg-transparent py-5'
    }`}>
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between">
          
          {/* Logo & Brand */}
          <a href="#" className="flex items-center gap-3 group">
            <div className="relative w-10 h-10 rounded-xl bg-gradient-to-tr from-brand-indigo to-brand-cyan p-0.5 shadow-lg shadow-brand-cyan/20 group-hover:shadow-brand-cyan/50 transition-all duration-300">
              <div className="w-full h-full bg-brand-dark rounded-[10px] flex items-center justify-center overflow-hidden">
                <img src="./logo.svg" alt="Voice Clear AI" className="w-7 h-7 object-contain transform group-hover:scale-110 transition-transform" />
              </div>
            </div>
            <div className="flex flex-col">
              <span className="font-extrabold text-lg sm:text-xl tracking-tight text-white flex items-center gap-1.5">
                Voice Clear <span className="text-gradient">AI</span>
              </span>
              <span className="text-[10px] font-mono text-slate-400 tracking-wider uppercase -mt-1">
                Real-Time Audio Engine
              </span>
            </div>
          </a>

          {/* Desktop Navigation */}
          <nav className="hidden md:flex items-center gap-6 lg:gap-8 text-sm font-medium text-slate-300">
            <a href="#features" className="hover:text-brand-cyan transition-colors">{t.nav.features}</a>
            <a href="#demo" className="hover:text-brand-cyan transition-colors">{t.nav.demo}</a>
            <a href="#simulator" className="hover:text-brand-cyan transition-colors">{t.nav.simulator}</a>
            <a href="#benchmarks" className="hover:text-brand-cyan transition-colors">{t.nav.benchmarks}</a>
            <a href="#pricing" className="hover:text-brand-cyan transition-colors">{t.nav.pricing}</a>
            <a href="#faq" className="hover:text-brand-cyan transition-colors">{t.nav.faq}</a>
          </nav>

          {/* Right Action Buttons */}
          <div className="hidden lg:flex items-center gap-3">
            {/* Language Switcher */}
            <button
              onClick={() => setLang(lang === 'es' ? 'en' : 'es')}
              className="flex items-center gap-1.5 px-3 py-1.5 rounded-lg bg-brand-card hover:bg-brand-cardHover border border-brand-border text-xs font-medium text-slate-300 transition-all"
              title="Cambiar idioma / Switch language"
            >
              <Globe className="w-3.5 h-3.5 text-brand-cyan" />
              <span className="font-mono uppercase font-bold">{lang}</span>
            </button>

            {/* WhatsApp CTA */}
            <button
              onClick={() => openWhatsApp()}
              className="flex items-center gap-1.5 px-4 py-2 rounded-xl bg-gradient-to-r from-[#25D366] to-[#128C7E] hover:opacity-95 text-xs font-bold text-white shadow-lg shadow-[#25D366]/20 transition-all transform hover:-translate-y-0.5"
            >
              <MessageCircle className="w-4 h-4 fill-current" />
              <span>{t.nav.buyPro}</span>
            </button>
          </div>

          {/* Mobile Menu Button */}
          <div className="flex md:hidden items-center gap-2">
            <button
              onClick={() => setLang(lang === 'es' ? 'en' : 'es')}
              className="p-2 rounded-lg bg-brand-card border border-brand-border text-xs font-bold text-slate-300 font-mono uppercase"
            >
              {lang}
            </button>
            <button
              onClick={() => setMobileMenuOpen(!mobileMenuOpen)}
              className="p-2 rounded-lg bg-brand-card border border-brand-border text-slate-300 hover:text-white"
            >
              {mobileMenuOpen ? <X className="w-6 h-6" /> : <Menu className="w-6 h-6" />}
            </button>
          </div>
        </div>
      </div>

      {/* Mobile Dropdown Menu */}
      {mobileMenuOpen && (
        <div className="md:hidden bg-brand-darker border-b border-brand-border px-4 pt-3 pb-6 space-y-3 mt-3 animate-fadeIn">
          <div className="flex flex-col space-y-2 text-base font-medium text-slate-300">
            <a 
              href="#features" 
              onClick={() => setMobileMenuOpen(false)}
              className="px-3 py-2 rounded-lg hover:bg-brand-card text-slate-200"
            >
              {t.nav.features}
            </a>
            <a 
              href="#demo" 
              onClick={() => setMobileMenuOpen(false)}
              className="px-3 py-2 rounded-lg hover:bg-brand-card text-slate-200"
            >
              {t.nav.demo}
            </a>
            <a 
              href="#simulator" 
              onClick={() => setMobileMenuOpen(false)}
              className="px-3 py-2 rounded-lg hover:bg-brand-card text-slate-200"
            >
              {t.nav.simulator}
            </a>
            <a 
              href="#benchmarks" 
              onClick={() => setMobileMenuOpen(false)}
              className="px-3 py-2 rounded-lg hover:bg-brand-card text-slate-200"
            >
              {t.nav.benchmarks}
            </a>
            <a 
              href="#pricing" 
              onClick={() => setMobileMenuOpen(false)}
              className="px-3 py-2 rounded-lg hover:bg-brand-card text-slate-200"
            >
              {t.nav.pricing}
            </a>
            <a 
              href="#faq" 
              onClick={() => setMobileMenuOpen(false)}
              className="px-3 py-2 rounded-lg hover:bg-brand-card text-slate-200"
            >
              {t.nav.faq}
            </a>
          </div>

          <div className="pt-4 border-t border-brand-border flex flex-col gap-2.5">
            <button
              onClick={() => { setMobileMenuOpen(false); openWhatsApp(); }}
              className="w-full py-3 rounded-xl bg-gradient-to-r from-[#25D366] to-[#128C7E] text-white font-bold text-sm flex items-center justify-center gap-2 shadow-lg shadow-[#25D366]/20"
            >
              <MessageCircle className="w-4 h-4 fill-current" />
              {t.nav.buyPro}
            </button>
          </div>
        </div>
      )}
    </header>
  );
}
