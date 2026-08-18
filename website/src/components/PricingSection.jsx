import React from 'react';
import { Check, Sparkles, ShieldCheck, Zap, Download, ShoppingBag, ArrowRight } from 'lucide-react';

export default function PricingSection({ t, onOpenDownload, onOpenCheckout }) {
  return (
    <section id="pricing" className="py-24 relative bg-brand-dark overflow-hidden">
      {/* Background glow behind featured card */}
      <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-[700px] h-[500px] bg-brand-cyan/10 rounded-full blur-[140px] pointer-events-none" />

      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 relative">
        
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

        {/* Pricing Cards Grid */}
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-8 items-stretch">
          {t.pricing.plans.map((plan) => {
            const isFeatured = plan.featured;
            return (
              <div
                key={plan.id}
                className={`relative rounded-3xl p-8 flex flex-col justify-between transition-all duration-300 ${
                  isFeatured
                    ? 'bg-gradient-to-b from-[#162138] to-[#0D1322] border-2 border-brand-cyan shadow-2xl shadow-brand-cyan/20 lg:-translate-y-3'
                    : 'glass-panel border border-brand-border hover:border-slate-700'
                }`}
              >
                {/* Popular Pill Badge */}
                {isFeatured && (
                  <div className="absolute -top-3.5 left-1/2 -translate-x-1/2 px-4 py-1 rounded-full bg-gradient-to-r from-brand-cyan to-brand-mint text-slate-950 text-xs font-extrabold tracking-wider uppercase font-mono shadow-md">
                    {t.pricing.popularTag}
                  </div>
                )}

                <div>
                  {/* Plan Name & Description */}
                  <h3 className="text-xl font-bold text-white mb-2">{plan.name}</h3>
                  <p className="text-xs sm:text-sm text-slate-400 mb-6 min-h-[40px] leading-relaxed">
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
                        <div className={`mt-0.5 rounded-full p-0.5 ${isFeatured ? 'bg-brand-cyan text-slate-950' : 'bg-slate-800 text-brand-cyan'}`}>
                          <Check className="w-3.5 h-3.5 stroke-[3]" />
                        </div>
                        <span>{feature}</span>
                      </div>
                    ))}
                  </div>
                </div>

                {/* Action CTA Button */}
                <div>
                  {plan.id === 'free' ? (
                    <button
                      onClick={onOpenDownload}
                      className="w-full py-4 rounded-2xl bg-slate-800 hover:bg-slate-700 border border-slate-700 text-white font-bold text-sm flex items-center justify-center gap-2 transition-all"
                    >
                      <Download className="w-4 h-4 text-brand-cyan" />
                      <span>{plan.buttonText}</span>
                    </button>
                  ) : (
                    <button
                      onClick={() => onOpenCheckout(plan.id)}
                      className={`w-full py-4 rounded-2xl font-bold text-sm flex items-center justify-center gap-2 transition-all transform hover:-translate-y-0.5 ${
                        isFeatured
                          ? 'bg-gradient-to-r from-brand-cyan via-brand-accent to-brand-mint text-slate-950 shadow-xl shadow-brand-cyan/25 hover:shadow-brand-cyan/40 font-extrabold'
                          : 'bg-brand-card hover:bg-brand-cardHover border border-brand-border text-white'
                      }`}
                    >
                      <ShoppingBag className="w-4 h-4" />
                      <span>{plan.buttonText}</span>
                    </button>
                  )}
                </div>

              </div>
            );
          })}
        </div>

        {/* Money back notice */}
        <div className="text-center mt-12 flex items-center justify-center gap-2 text-xs sm:text-sm text-slate-400 font-medium">
          <ShieldCheck className="w-5 h-5 text-brand-mint" />
          <span>{t.pricing.moneyBack}</span>
        </div>

      </div>
    </section>
  );
}
