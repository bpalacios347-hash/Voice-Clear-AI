import React from 'react';
import { Check, Sparkles, ShieldCheck, Headphones, MessageCircle, ArrowRight } from 'lucide-react';

export default function PricingSection({ t }) {
  const handleWhatsAppRedirect = (msg) => {
    const defaultMsg = msg || "Hola, quiero adquirir la licencia de Voice Clear AI.";
    const url = `https://wa.me/50587414791?text=${encodeURIComponent(defaultMsg)}`;
    window.open(url, '_blank');
  };

  return (
    <section id="pricing" className="py-24 relative bg-brand-dark overflow-hidden">
      {/* Background glow behind cards */}
      <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-[600px] h-[450px] bg-brand-cyan/10 rounded-full blur-[140px] pointer-events-none" />

      <div className="max-w-6xl mx-auto px-4 sm:px-6 lg:px-8 relative">
        
        {/* Header */}
        <div className="text-center max-w-3xl mx-auto mb-16">
          <span className="text-xs font-mono font-bold tracking-widest text-brand-cyan uppercase px-3 py-1 rounded-full bg-brand-cyan/10 border border-brand-cyan/20">
            {t.pricing.tag}
          </span>
          <h2 className="text-3xl sm:text-4xl lg:text-5xl font-black text-white mt-4 mb-4 tracking-tight">
            {t.pricing.title}
          </h2>
          <p className="text-slate-300 text-base sm:text-lg">
            {t.pricing.subtitle}
          </p>
        </div>

        {/* Pricing Cards Grid (2 Plans: $6.99 and $10) */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-8 max-w-4xl mx-auto items-stretch">
          {t.pricing.plans.map((plan) => {
            const isFeatured = plan.featured;
            return (
              <div
                key={plan.id}
                className={`relative rounded-3xl p-8 sm:p-10 flex flex-col justify-between transition-all duration-300 ${
                  isFeatured
                    ? 'bg-gradient-to-b from-[#18233C] to-[#0D1322] border-2 border-brand-cyan shadow-2xl shadow-brand-cyan/25 md:-translate-y-2'
                    : 'glass-panel border border-brand-border hover:border-slate-700'
                }`}
              >
                {/* Popular / Best Value Badge */}
                {isFeatured && (
                  <div className="absolute -top-3.5 left-1/2 -translate-x-1/2 px-4 py-1 rounded-full bg-gradient-to-r from-brand-cyan to-brand-mint text-slate-950 text-xs font-extrabold tracking-wider uppercase font-mono shadow-md flex items-center gap-1.5">
                    <Sparkles className="w-3.5 h-3.5" />
                    <span>{t.pricing.popularTag}</span>
                  </div>
                )}

                <div>
                  {/* Plan Name & Description */}
                  <h3 className="text-2xl font-bold text-white mb-2">{plan.name}</h3>
                  <p className="text-xs sm:text-sm text-slate-300 mb-6 min-h-[38px] leading-relaxed">
                    {plan.desc}
                  </p>

                  {/* Price */}
                  <div className="flex items-baseline gap-2 mb-1">
                    <span className="text-4xl sm:text-5xl font-black font-mono text-white tracking-tight">
                      {plan.price}
                    </span>
                    {plan.oldPrice && (
                      <span className="text-lg line-through text-slate-500 font-mono">
                        {plan.oldPrice}
                      </span>
                    )}
                    <span className="text-xs text-slate-400 font-mono">
                      USD
                    </span>
                  </div>
                  <span className="text-xs font-mono text-brand-mint font-semibold block mb-8">
                    {plan.period}
                  </span>

                  {/* Features List */}
                  <div className="space-y-3.5 pt-6 border-t border-slate-800 mb-8">
                    {plan.features.map((feature, fIdx) => (
                      <div key={fIdx} className="flex items-start gap-3 text-xs sm:text-sm text-slate-200">
                        <div className={`mt-0.5 rounded-full p-0.5 shrink-0 ${isFeatured ? 'bg-brand-cyan text-slate-950' : 'bg-slate-800 text-brand-cyan'}`}>
                          <Check className="w-3.5 h-3.5 stroke-[3]" />
                        </div>
                        <span>{feature}</span>
                      </div>
                    ))}
                  </div>
                </div>

                {/* WhatsApp Action Button */}
                <div>
                  <button
                    onClick={() => handleWhatsAppRedirect(plan.whatsappMsg)}
                    className={`w-full py-4 rounded-2xl font-bold text-sm sm:text-base flex items-center justify-center gap-2.5 transition-all transform hover:-translate-y-0.5 ${
                      isFeatured
                        ? 'bg-gradient-to-r from-[#25D366] to-[#128C7E] hover:opacity-95 text-white shadow-xl shadow-[#25D366]/25 font-extrabold'
                        : 'bg-[#25D366]/20 hover:bg-[#25D366]/30 border border-[#25D366]/50 text-[#25D366] font-bold'
                    }`}
                  >
                    <MessageCircle className="w-5 h-5 fill-current" />
                    <span>{plan.buttonText}</span>
                  </button>
                  <span className="block text-[11px] font-mono text-slate-400 text-center mt-2">
                    Contacto directo: +505 8741 4791
                  </span>
                </div>

              </div>
            );
          })}
        </div>

        {/* Guarantees */}
        <div className="mt-14 pt-8 border-t border-slate-800/80 flex items-center justify-center gap-2 text-center text-xs sm:text-sm text-slate-300 font-medium">
          <ShieldCheck className="w-5 h-5 text-brand-mint shrink-0" />
          <span>{t.pricing.moneyBack}</span>
        </div>

      </div>
    </section>
  );
}
