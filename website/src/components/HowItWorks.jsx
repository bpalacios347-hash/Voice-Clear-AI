import React from 'react';
import { Download, Sliders, Mic, ArrowRight } from 'lucide-react';

export default function HowItWorks({ t }) {
  const stepIcons = [Download, Sliders, Mic];

  return (
    <section className="py-24 relative bg-brand-darker">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        
        {/* Header */}
        <div className="text-center max-w-3xl mx-auto mb-16">
          <span className="text-xs font-mono font-bold tracking-widest text-brand-mint uppercase px-3 py-1 rounded-full bg-brand-mint/10 border border-brand-mint/20">
            {t.howItWorks.tag}
          </span>
          <h2 className="text-3xl sm:text-4xl lg:text-5xl font-black text-white mt-4 mb-4 tracking-tight">
            {t.howItWorks.title}
          </h2>
          <p className="text-slate-300 text-base sm:text-lg">
            {t.howItWorks.subtitle}
          </p>
        </div>

        {/* Steps Grid */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-8 relative">
          {t.howItWorks.steps.map((step, idx) => {
            const Icon = stepIcons[idx] || Mic;
            return (
              <div 
                key={idx}
                className="glass-card-glow rounded-3xl p-8 border border-brand-border relative flex flex-col justify-between group hover:border-brand-cyan/50 transition-all duration-300"
              >
                <div>
                  {/* Step Number & Icon */}
                  <div className="flex items-center justify-between mb-8">
                    <span className="text-4xl font-black font-mono text-slate-700 group-hover:text-brand-cyan transition-colors">
                      {step.step}
                    </span>
                    <div className="w-12 h-12 rounded-2xl bg-brand-cyan/10 border border-brand-cyan/20 flex items-center justify-center">
                      <Icon className="w-6 h-6 text-brand-cyan" />
                    </div>
                  </div>

                  {/* Title */}
                  <h3 className="text-xl font-bold text-white mb-3 tracking-tight">
                    {step.title}
                  </h3>

                  {/* Description */}
                  <p className="text-sm text-slate-300 leading-relaxed">
                    {step.desc}
                  </p>
                </div>

                <div className="pt-6 mt-6 border-t border-slate-800/80 flex items-center text-xs font-mono text-brand-mint font-semibold">
                  <span>Paso {idx + 1} de 3</span>
                </div>
              </div>
            );
          })}
        </div>

      </div>
    </section>
  );
}
