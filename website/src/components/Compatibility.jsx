import React from 'react';
import { CheckCircle2, Radio, Video, Headphones, MessageSquare, Cast } from 'lucide-react';

export default function Compatibility({ t }) {
  return (
    <section className="py-20 relative bg-brand-dark border-y border-brand-border">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        
        {/* Header */}
        <div className="text-center max-w-3xl mx-auto mb-14">
          <span className="text-xs font-mono font-bold tracking-widest text-brand-cyan uppercase px-3 py-1 rounded-full bg-brand-cyan/10 border border-brand-cyan/20">
            {t.compatibility.tag}
          </span>
          <h2 className="text-3xl sm:text-4xl font-black text-white mt-3 mb-3 tracking-tight">
            {t.compatibility.title}
          </h2>
          <p className="text-slate-300 text-base">
            {t.compatibility.subtitle}
          </p>
        </div>

        {/* Compatibility App Cards */}
        <div className="grid grid-cols-2 sm:grid-cols-4 gap-4 sm:gap-6">
          {t.compatibility.apps.map((app, idx) => (
            <div 
              key={idx}
              className="glass-panel p-5 rounded-2xl flex flex-col items-center justify-center text-center group hover:border-brand-cyan/50 hover:bg-brand-cardHover transition-all duration-300"
            >
              <div className="w-12 h-12 rounded-xl bg-slate-900 border border-slate-800 flex items-center justify-center mb-3 group-hover:border-brand-cyan/40 transition-colors">
                <CheckCircle2 className="w-6 h-6 text-brand-mint" />
              </div>
              <h3 className="font-bold text-base text-white group-hover:text-brand-cyan transition-colors">
                {app.name}
              </h3>
              <span className="text-xs text-slate-400 font-mono mt-1">
                {app.category}
              </span>
            </div>
          ))}
        </div>

      </div>
    </section>
  );
}
