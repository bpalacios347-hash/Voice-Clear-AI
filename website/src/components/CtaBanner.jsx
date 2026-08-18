import React from 'react';
import { Download, MessageCircle, Sparkles } from 'lucide-react';

export default function CtaBanner({ t }) {
  const openWhatsAppSingle = () => {
    const text = "Hola, quiero comprar la licencia de Voice Clear AI (1 Dispositivo - $6.99 USD).";
    window.open(`https://wa.me/50587414791?text=${encodeURIComponent(text)}`, '_blank');
  };

  const openWhatsAppCombo = () => {
    const text = "Hola, quiero comprar el Combo de Voice Clear AI (2 Dispositivos - $10 USD).";
    window.open(`https://wa.me/50587414791?text=${encodeURIComponent(text)}`, '_blank');
  };

  return (
    <section className="py-20 relative bg-brand-darker overflow-hidden">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 relative">
        
        <div className="relative rounded-3xl p-8 sm:p-14 overflow-hidden border border-brand-cyan/40 bg-gradient-to-r from-brand-indigo/40 via-brand-dark to-brand-cyan/20 shadow-2xl">
          {/* Background decorative wave rings */}
          <div className="absolute -right-20 -bottom-20 w-80 h-80 rounded-full bg-brand-cyan/10 blur-3xl pointer-events-none" />

          <div className="relative z-10 max-w-3xl">
            <span className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-brand-mint/10 border border-brand-mint/30 text-brand-mint text-xs font-mono font-bold uppercase mb-4">
              <Sparkles className="w-3.5 h-3.5" />
              Windows 10 / 11 64-bit • Instalador Oficial
            </span>
            <h2 className="text-3xl sm:text-4xl lg:text-5xl font-black text-white tracking-tight mb-4">
              {t.ctaBanner.title}
            </h2>
            <p className="text-slate-300 text-base sm:text-lg mb-8 leading-relaxed">
              {t.ctaBanner.subtitle}
            </p>

            <div className="flex flex-col sm:flex-row items-center gap-4">
              <a
                href="./VoiceClearAI_Setup.exe"
                download="VoiceClearAI_Setup.exe"
                className="w-full sm:w-auto flex items-center justify-center gap-2.5 px-8 py-4 rounded-2xl bg-white hover:bg-slate-100 text-slate-950 font-extrabold text-base shadow-xl transition-all transform hover:-translate-y-0.5"
              >
                <Download className="w-5 h-5 text-brand-indigo" />
                <span>{t.ctaBanner.buttonDownload}</span>
              </a>

              <button
                onClick={openWhatsAppSingle}
                className="w-full sm:w-auto flex items-center justify-center gap-2.5 px-8 py-4 rounded-2xl bg-gradient-to-r from-[#25D366] to-[#128C7E] text-white font-extrabold text-base shadow-xl shadow-[#25D366]/30 hover:opacity-95 transition-all transform hover:-translate-y-0.5"
              >
                <MessageCircle className="w-5 h-5 fill-current" />
                <span>{t.ctaBanner.buttonSingle}</span>
              </button>
            </div>
          </div>

        </div>

      </div>
    </section>
  );
}
