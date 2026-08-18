import React from 'react';
import { Download, ShieldCheck, Zap, Cpu, Sparkles, CheckCircle2, MessageCircle } from 'lucide-react';

export default function Hero({ t }) {
  const openWhatsAppSingle = () => {
    const text = "Hola, quiero comprar la licencia de Voice Clear AI (1 Dispositivo - $6.99 USD).";
    window.open(`https://wa.me/50587414791?text=${encodeURIComponent(text)}`, '_blank');
  };

  const openWhatsAppCombo = () => {
    const text = "Hola, quiero comprar el Combo de Voice Clear AI (2 Dispositivos - $10 USD).";
    window.open(`https://wa.me/50587414791?text=${encodeURIComponent(text)}`, '_blank');
  };

  return (
    <section className="relative pt-32 pb-20 lg:pt-40 lg:pb-32 overflow-hidden bg-radial-glow">
      {/* Background Decorative Grid & Glow Elements */}
      <div className="absolute inset-0 bg-grid-pattern opacity-40 pointer-events-none" />
      <div className="absolute top-1/4 left-1/2 -translate-x-1/2 -translate-y-1/2 w-[600px] h-[600px] bg-gradient-to-tr from-brand-indigo/20 via-brand-cyan/20 to-brand-mint/10 rounded-full blur-3xl pointer-events-none" />
      
      <div className="relative max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex flex-col items-center text-center max-w-4xl mx-auto">
          
          {/* Release Version Badge */}
          <div className="inline-flex items-center gap-2 px-4 py-1.5 rounded-full bg-brand-card/80 border border-brand-border text-slate-300 text-xs sm:text-sm font-medium mb-8 backdrop-blur-md shadow-inner animate-float">
            <span className="flex h-2 w-2 rounded-full bg-brand-cyan animate-ping" />
            <span className="text-brand-cyan font-mono font-semibold">v1.0.0</span>
            <span className="text-slate-400">|</span>
            <span>{t.hero.badge}</span>
            <Sparkles className="w-3.5 h-3.5 text-brand-mint" />
          </div>

          {/* Main Headline */}
          <h1 className="text-4xl sm:text-6xl lg:text-7xl font-black tracking-tight text-white leading-[1.1] mb-6">
            {t.hero.titleStart}{" "}
            <span className="text-gradient drop-shadow-sm">{t.hero.titleHighlight}</span>
            <br />
            <span className="text-white font-extrabold">{t.hero.titleEnd}</span>
          </h1>

          {/* Subtitle */}
          <p className="text-base sm:text-lg lg:text-xl text-slate-300 max-w-3xl mb-10 leading-relaxed font-normal">
            {t.hero.subtitle}
          </p>

          {/* CTA Action Buttons Group */}
          <div className="flex flex-col sm:flex-row items-center justify-center gap-4 w-full sm:w-auto mb-8">
            
            {/* Direct .EXE Download Button */}
            <a
              href="./VoiceClearAI_Setup.exe"
              download="VoiceClearAI_Setup.exe"
              className="w-full sm:w-auto flex items-center justify-center gap-3 px-8 py-4 rounded-2xl bg-gradient-to-r from-brand-cyan via-brand-accent to-brand-mint text-slate-950 font-extrabold text-base shadow-xl shadow-brand-cyan/25 hover:shadow-brand-cyan/40 transform hover:-translate-y-1 transition-all duration-200 group"
            >
              <Download className="w-5 h-5 group-hover:animate-bounce" />
              <div className="flex flex-col text-left">
                <span>{t.hero.ctaDownload}</span>
                <span className="text-[10px] font-mono font-medium text-slate-900 tracking-tight opacity-90">
                  {t.hero.ctaDownloadSub} (~75 MB)
                </span>
              </div>
            </a>

            {/* Buy 1 PC WhatsApp Button */}
            <button
              onClick={openWhatsAppSingle}
              className="w-full sm:w-auto flex items-center justify-center gap-2.5 px-6 py-4 rounded-2xl bg-gradient-to-r from-[#25D366] to-[#128C7E] text-white font-extrabold text-sm sm:text-base shadow-lg shadow-[#25D366]/20 hover:opacity-95 transform hover:-translate-y-1 transition-all"
            >
              <MessageCircle className="w-5 h-5 fill-current" />
              <span>{t.hero.ctaSingle}</span>
            </button>

            {/* Combo 2 PCs WhatsApp Button */}
            <button
              onClick={openWhatsAppCombo}
              className="w-full sm:w-auto flex items-center justify-center gap-2 px-6 py-4 rounded-2xl bg-brand-card hover:bg-brand-cardHover border border-brand-cyan/40 text-white font-bold text-sm sm:text-base shadow-lg transition-all transform hover:-translate-y-1"
            >
              <Sparkles className="w-4 h-4 text-brand-mint" />
              <span>{t.hero.ctaCombo}</span>
            </button>
          </div>

          {/* Trust Guarantees */}
          <div className="flex flex-wrap items-center justify-center gap-y-2 gap-x-6 text-xs text-slate-400 font-medium mb-16">
            <div className="flex items-center gap-1.5">
              <CheckCircle2 className="w-4 h-4 text-brand-mint" />
              <span>Garantía de 5 Días</span>
            </div>
            <div className="flex items-center gap-1.5">
              <CheckCircle2 className="w-4 h-4 text-brand-cyan" />
              <span>1 Año de Soporte Técnico</span>
            </div>
            <div className="flex items-center gap-1.5">
              <ShieldCheck className="w-4 h-4 text-brand-cyan" />
              <span>100% On-Device (Sin Nube)</span>
            </div>
            <div className="flex items-center gap-1.5">
              <Zap className="w-4 h-4 text-yellow-400" />
              <span>&lt; 10ms Latencia</span>
            </div>
            <div className="flex items-center gap-1.5">
              <Cpu className="w-4 h-4 text-purple-400" />
              <span>0% GPU Requerida</span>
            </div>
          </div>

          {/* Real Benchmark Stats Cards */}
          <div className="w-full grid grid-cols-2 md:grid-cols-4 gap-3 sm:gap-4 max-w-5xl mx-auto">
            {t.hero.stats.map((stat, idx) => (
              <div 
                key={idx}
                className="glass-panel rounded-2xl p-4 sm:p-5 flex flex-col items-center justify-center relative overflow-hidden group hover:border-brand-cyan/40 transition-all duration-300"
              >
                <div className="absolute inset-0 bg-gradient-to-b from-brand-cyan/5 to-transparent opacity-0 group-hover:opacity-100 transition-opacity" />
                <span className="text-2xl sm:text-3xl lg:text-4xl font-black text-white font-mono tracking-tight group-hover:text-brand-cyan transition-colors">
                  {stat.value}
                </span>
                <span className="text-xs sm:text-sm text-slate-400 font-medium mt-1 text-center">
                  {stat.label}
                </span>
              </div>
            ))}
          </div>

        </div>
      </div>
    </section>
  );
}
