import React from 'react';
import { Github, Shield, Heart } from 'lucide-react';

export default function Footer({ t }) {
  return (
    <footer className="bg-brand-darker border-t border-brand-border py-16 text-slate-400 text-sm">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        
        <div className="grid grid-cols-1 md:grid-cols-5 gap-10 mb-12">
          
          {/* Logo & Tagline */}
          <div className="md:col-span-2 space-y-4">
            <div className="flex items-center gap-3">
              <div className="w-8 h-8 rounded-lg bg-gradient-to-tr from-brand-indigo to-brand-cyan p-0.5 shadow-md">
                <div className="w-full h-full bg-brand-dark rounded-[6px] flex items-center justify-center">
                  <img src="./logo.svg" alt="Voice Clear AI" className="w-5 h-5 object-contain" />
                </div>
              </div>
              <span className="font-extrabold text-lg text-white tracking-tight">
                Voice Clear <span className="text-gradient">AI</span>
              </span>
            </div>
            <p className="text-xs sm:text-sm text-slate-400 max-w-sm leading-relaxed">
              {t.footer.tagline}
            </p>
            <div className="pt-2">
              <a
                href="https://github.com/bpalacios347-hash/Voice-Clear-AI"
                target="_blank"
                rel="noopener noreferrer"
                className="inline-flex items-center gap-2 px-3 py-1.5 rounded-lg bg-brand-card hover:bg-brand-cardHover border border-brand-border text-xs font-mono text-slate-300 transition-colors"
              >
                <Github className="w-4 h-4 text-brand-cyan" />
                <span>github.com/bpalacios347-hash/Voice-Clear-AI</span>
              </a>
            </div>
          </div>

          {/* Column: Product */}
          <div className="space-y-3">
            <h4 className="text-xs font-mono font-bold text-white uppercase tracking-wider">
              {t.footer.product}
            </h4>
            <ul className="space-y-2 text-xs sm:text-sm">
              <li><a href="#features" className="hover:text-brand-cyan transition-colors">{t.footer.features}</a></li>
              <li><a href="#demo" className="hover:text-brand-cyan transition-colors">{t.footer.demo}</a></li>
              <li><a href="#pricing" className="hover:text-brand-cyan transition-colors">{t.footer.pricing}</a></li>
              <li><a href="#simulator" className="hover:text-brand-cyan transition-colors">Simulador Windows</a></li>
            </ul>
          </div>

          {/* Column: Resources */}
          <div className="space-y-3">
            <h4 className="text-xs font-mono font-bold text-white uppercase tracking-wider">
              {t.footer.resources}
            </h4>
            <ul className="space-y-2 text-xs sm:text-sm">
              <li>
                <a 
                  href="https://github.com/bpalacios347-hash/Voice-Clear-AI/blob/main/docs/ArchitectureOverview.md" 
                  target="_blank" 
                  rel="noopener noreferrer" 
                  className="hover:text-brand-cyan transition-colors"
                >
                  {t.footer.docs}
                </a>
              </li>
              <li>
                <a 
                  href="https://github.com/bpalacios347-hash/Voice-Clear-AI/blob/main/benchmark_results.json" 
                  target="_blank" 
                  rel="noopener noreferrer" 
                  className="hover:text-brand-cyan transition-colors"
                >
                  {t.footer.benchmarks}
                </a>
              </li>
              <li>
                <a 
                  href="https://github.com/bpalacios347-hash/Voice-Clear-AI/blob/main/docs/ReleaseNotes.md" 
                  target="_blank" 
                  rel="noopener noreferrer" 
                  className="hover:text-brand-cyan transition-colors"
                >
                  {t.footer.releaseNotes}
                </a>
              </li>
            </ul>
          </div>

          {/* Column: Legal & Tech */}
          <div className="space-y-3">
            <h4 className="text-xs font-mono font-bold text-white uppercase tracking-wider">
              {t.footer.legal}
            </h4>
            <ul className="space-y-2 text-xs sm:text-sm">
              <li><span className="hover:text-slate-300 cursor-pointer">{t.footer.privacy}</span></li>
              <li><span className="hover:text-slate-300 cursor-pointer">{t.footer.terms}</span></li>
              <li><span className="hover:text-slate-300 cursor-pointer">{t.footer.license} (MIT)</span></li>
            </ul>
          </div>

        </div>

        {/* Bottom copyright line */}
        <div className="pt-8 border-t border-slate-800/80 flex flex-col sm:flex-row items-center justify-between gap-4 text-xs text-slate-400">
          <p>{t.footer.copyright}</p>
          <div className="flex items-center gap-1 text-slate-400">
            <span>Powered by DeepFilterNet3, ONNX Runtime & AVStream WDK</span>
          </div>
        </div>

      </div>
    </footer>
  );
}
