import React from 'react';
import { ShieldCheck, Zap, Cpu, Mic, Sparkles, Layers } from 'lucide-react';

const iconMap = {
  ShieldCheck,
  Zap,
  Cpu,
  Mic,
  Sparkles,
  Layers
};

export default function Features({ t }) {
  return (
    <section id="features" className="py-24 relative bg-brand-dark overflow-hidden">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 relative">
        
        {/* Header */}
        <div className="text-center max-w-3xl mx-auto mb-16">
          <span className="text-xs font-mono font-bold tracking-widest text-brand-cyan uppercase px-3 py-1 rounded-full bg-brand-cyan/10 border border-brand-cyan/20">
            {t.features.tag}
          </span>
          <h2 className="text-3xl sm:text-4xl lg:text-5xl font-black text-white mt-4 mb-4 tracking-tight">
            {t.features.title}
          </h2>
          <p className="text-slate-300 text-base sm:text-lg">
            {t.features.subtitle}
          </p>
        </div>

        {/* Feature Cards Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6 sm:gap-8">
          {t.features.items.map((item, idx) => {
            const IconComponent = iconMap[item.icon] || Sparkles;
            return (
              <div 
                key={idx}
                className="glass-panel glass-panel-hover rounded-3xl p-6 sm:p-8 flex flex-col justify-between transition-all duration-300 group"
              >
                <div>
                  {/* Icon with glowing backdrop */}
                  <div className="w-14 h-14 rounded-2xl bg-gradient-to-tr from-brand-indigo/30 via-brand-cyan/20 to-brand-mint/20 border border-brand-border flex items-center justify-center mb-6 group-hover:border-brand-cyan/60 group-hover:scale-110 transition-all duration-300">
                    <IconComponent className="w-7 h-7 text-brand-cyan group-hover:text-brand-mint transition-colors" />
                  </div>

                  {/* Title */}
                  <h3 className="text-xl font-bold text-white mb-3 tracking-tight group-hover:text-brand-cyan transition-colors">
                    {item.title}
                  </h3>

                  {/* Description */}
                  <p className="text-sm text-slate-300 leading-relaxed font-normal">
                    {item.desc}
                  </p>
                </div>

                {/* Bottom accent line */}
                <div className="w-12 h-1 bg-gradient-to-r from-brand-cyan to-transparent rounded-full mt-6 opacity-40 group-hover:w-full group-hover:opacity-100 transition-all duration-500" />
              </div>
            );
          })}
        </div>

      </div>
    </section>
  );
}
