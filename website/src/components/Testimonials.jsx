import React from 'react';
import { Star, Quote } from 'lucide-react';

export default function Testimonials({ t }) {
  return (
    <section className="py-24 relative bg-brand-darker">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        
        {/* Header */}
        <div className="text-center max-w-3xl mx-auto mb-16">
          <span className="text-xs font-mono font-bold tracking-widest text-brand-purple uppercase px-3 py-1 rounded-full bg-brand-purple/10 border border-brand-purple/20">
            {t.testimonials.tag}
          </span>
          <h2 className="text-3xl sm:text-4xl font-black text-white mt-4 mb-4 tracking-tight">
            {t.testimonials.title}
          </h2>
        </div>

        {/* Testimonials Cards Grid */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-8">
          {t.testimonials.items.map((item, idx) => (
            <div
              key={idx}
              className="glass-card-glow rounded-3xl p-8 border border-brand-border flex flex-col justify-between hover:border-brand-cyan/40 transition-all duration-300 relative group"
            >
              <div>
                {/* 5-star rating */}
                <div className="flex items-center gap-1 mb-6 text-amber-400">
                  {[...Array(5)].map((_, i) => (
                    <Star key={i} className="w-4 h-4 fill-current" />
                  ))}
                </div>

                {/* Comment */}
                <p className="text-slate-300 text-sm leading-relaxed mb-6 italic">
                  "{item.comment}"
                </p>
              </div>

              {/* User info */}
              <div className="flex items-center gap-3 pt-6 border-t border-slate-800/80">
                <img
                  src={item.avatar}
                  alt={item.name}
                  className="w-11 h-11 rounded-full object-cover border border-brand-cyan/40"
                />
                <div>
                  <h4 className="font-bold text-sm text-white">{item.name}</h4>
                  <p className="text-xs text-slate-400 font-mono">{item.role}</p>
                </div>
              </div>
            </div>
          ))}
        </div>

      </div>
    </section>
  );
}
