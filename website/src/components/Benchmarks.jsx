import React from 'react';
import { Award, Zap, Shield, Check, X, Sparkles } from 'lucide-react';

export default function Benchmarks({ t }) {
  return (
    <section id="benchmarks" className="py-24 relative bg-brand-darker">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        
        {/* Header */}
        <div className="text-center max-w-3xl mx-auto mb-16">
          <span className="text-xs font-mono font-bold tracking-widest text-brand-purple uppercase px-3 py-1 rounded-full bg-brand-purple/10 border border-brand-purple/20">
            {t.benchmarks.tag}
          </span>
          <h2 className="text-3xl sm:text-4xl lg:text-5xl font-black text-white mt-4 mb-4 tracking-tight">
            {t.benchmarks.title}
          </h2>
          <p className="text-slate-300 text-base sm:text-lg">
            {t.benchmarks.subtitle}
          </p>
        </div>

        {/* Comparison Table Card */}
        <div className="glass-card-glow rounded-3xl border border-brand-border overflow-hidden shadow-2xl">
          <div className="overflow-x-auto">
            <table className="w-full text-left border-collapse">
              <thead>
                <tr className="border-b border-brand-border bg-brand-darker/90">
                  <th className="p-4 sm:p-6 text-xs sm:text-sm font-mono font-semibold text-slate-400 uppercase tracking-wider">
                    {t.benchmarks.tableHeaders.feature}
                  </th>
                  <th className="p-4 sm:p-6 text-xs sm:text-sm font-mono font-bold text-brand-cyan bg-brand-cyan/10 border-x border-brand-cyan/30 text-center">
                    <div className="flex items-center justify-center gap-1.5">
                      <Sparkles className="w-4 h-4 text-brand-mint" />
                      <span>{t.benchmarks.tableHeaders.voiceclear}</span>
                    </div>
                  </th>
                  <th className="p-4 sm:p-6 text-xs sm:text-sm font-mono font-semibold text-slate-300 text-center">
                    {t.benchmarks.tableHeaders.krisp}
                  </th>
                  <th className="p-4 sm:p-6 text-xs sm:text-sm font-mono font-semibold text-slate-300 text-center">
                    {t.benchmarks.tableHeaders.rtx}
                  </th>
                  <th className="p-4 sm:p-6 text-xs sm:text-sm font-mono font-semibold text-slate-300 text-center">
                    {t.benchmarks.tableHeaders.discord}
                  </th>
                </tr>
              </thead>
              <tbody className="divide-y divide-brand-border text-xs sm:text-sm">
                {t.benchmarks.rows.map((row, idx) => (
                  <tr key={idx} className="hover:bg-brand-card/40 transition-colors">
                    <td className="p-4 sm:p-6 font-semibold text-slate-200">
                      {row.metric}
                    </td>
                    
                    {/* Voice Clear AI column (Highlighted) */}
                    <td className="p-4 sm:p-6 text-center font-bold text-brand-mint bg-brand-cyan/5 border-x border-brand-cyan/20">
                      <span className="inline-flex items-center gap-1.5 px-3 py-1 rounded-full bg-brand-cyan/20 text-brand-cyan font-mono border border-brand-cyan/30">
                        {row.vc}
                      </span>
                    </td>

                    <td className="p-4 sm:p-6 text-center text-slate-400 font-mono">
                      {row.krisp}
                    </td>
                    <td className="p-4 sm:p-6 text-center text-slate-400 font-mono">
                      {row.rtx}
                    </td>
                    <td className="p-4 sm:p-6 text-center text-slate-400 font-mono">
                      {row.discord}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>

        {/* Small footnotes */}
        <p className="text-center text-xs text-slate-400 font-mono mt-6">
          * Mediciones basadas en el pipeline ONNX Runtime con bloques de 480 muestras a 48,000 Hz en procesadores Intel Core 8th Gen+ y AMD Ryzen.
        </p>

      </div>
    </section>
  );
}
