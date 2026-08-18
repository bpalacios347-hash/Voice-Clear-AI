export const translations = {
  es: {
    nav: {
      features: "Características",
      demo: "Demostración A/B",
      simulator: "Simulador App",
      benchmarks: "Rendimiento",
      pricing: "Precios",
      faq: "Preguntas",
      downloadFree: "Descargar Gratis",
      buyPro: "Comprar por WhatsApp ($6.99)",
      contact: "Contacto"
    },
    hero: {
      badge: "Voice Clear AI v1.0 — Motor DeepFilterNet3 en CPU",
      titleStart: "Tu Voz.",
      titleHighlight: "Cero Ruido.",
      titleEnd: "100% Privado en tu PC.",
      subtitle: "Elimina teclados mecánicos, ventiladores, ecos y ruidos de fondo en tiempo real con IA. Menos de 10ms de latencia, 0% de uso de GPU y sin enviar un solo byte de audio a la nube.",
      ctaDownload: "Descargar Prueba Gratis",
      ctaDownloadSub: "Windows 10 / 11 (64-bit)",
      ctaPricing: "Obtener Licencia ($6.99)",
      stats: [
        { label: "Reducción de Ruido", value: "-51.5 dB" },
        { label: "Latencia en Tiempo Real", value: "9.8 ms" },
        { label: "Calidad de Audio", value: "48 kHz HD" },
        { label: "Procesamiento", value: "100% On-Device" }
      ]
    },
    comparator: {
      tag: "COMPRUEBA LA DIFERENCIA",
      title: "Escucha el poder de la IA en tiempo real",
      subtitle: "Haz clic en reproducir y cambia entre el audio original con ruido de oficina/teclado y el audio procesado por Voice Clear AI.",
      playing: "Reproduciendo",
      paused: "Pausado",
      modeNoisy: "Audio Original (Con Ruido)",
      modeClean: "Voice Clear AI (Limpio)",
      clickToToggle: "Conmuta en vivo para comparar",
      sliderHint: "Arrastra la barra de tiempo o haz clic para adelantar",
      specsBanner: {
        attenuation: "51.58 dB Atenuación Acústica",
        engine: "ONNX Runtime + AVX2 oneDNN",
        latency: "9.84ms Tiempo de Procesamiento"
      }
    },
    simulator: {
      tag: "EXPERIENCIA DE USUARIO",
      title: "Interfaz ligera, elegante y lista en un clic",
      subtitle: "Construida con Qt 6 moderno y conectada a un servicio de Windows en segundo plano. Configúralo una vez y olvídate.",
      statusActive: "PROTECCIÓN ACTIVA",
      statusInactive: "PROTECCIÓN PAUSADA",
      toggleHint: "Haz clic en el botón del micrófono para alternar el filtro",
      micSelect: "Micrófono Físico de Entrada",
      virtualDevice: "Dispositivo Virtual de Salida",
      virtualDeviceName: "Voice Clear Virtual Mic (AVStream WDK)",
      noiseSuppressionLevel: "Nivel de Supresión de IA",
      cpuUsage: "Uso de CPU",
      latencyText: "Latencia Estimada",
      runInBackground: "Ejecutar como servicio silencioso de Windows en el inicio",
      autoStartActive: "Activo en segundo plano"
    },
    features: {
      tag: "POR QUÉ VOICE CLEAR AI",
      title: "Diseñado para máxima claridad y cero distracciones",
      subtitle: "La combinación perfecta entre inteligencia artificial acústica de última generación y optimización nativa para Windows.",
      items: [
        {
          icon: "ShieldCheck",
          title: "100% Local y Privacidad Total",
          desc: "Tus conversaciones nunca salen de tu ordenador. El modelo DeepFilterNet3 corre completamente en tu CPU local, cumpliendo con los estándares más estrictos de privacidad corporativa y médica."
        },
        {
          icon: "Zap",
          title: "Ultra-Baja Latencia (<10ms)",
          desc: "Con un pipeline optimizado en C++ y buffers lock-free SPSC de baja latencia, tu voz llega a Discord, Twitch, Zoom o Boostlingo sin ningún desfase o eco perceptible."
        },
        {
          icon: "Cpu",
          title: "0% Uso de GPU (AVX2 / oneDNN)",
          desc: "No necesitas una tarjeta gráfica cara como RTX. Diseñado específicamente para procesadores Intel Core (8va gen+) y AMD Ryzen con aceleración vectorial SIMD."
        },
        {
          icon: "Mic",
          title: "Driver de Micrófono Virtual Nativo",
          desc: "Incluye un driver de audio WDK AVStream certificado para Windows. Cualquier software que acepte un micrófono reconocerá 'Voice Clear Virtual Mic' automáticamente."
        },
        {
          icon: "Sparkles",
          title: "Fidelidad de Estudio 48 kHz",
          desc: "A diferencia de otros supresores que ahogan o metalizan tu voz, Voice Clear AI preserva el timbre, la calidez y los armónicos naturales a 48,000 muestras por segundo."
        },
        {
          icon: "Layers",
          title: "Servicio de Windows Silencioso",
          desc: "La arquitectura separa la interfaz gráfica (Qt 6) del servicio de audio en tiempo real. Puedes cerrar la ventana y el audio seguirá limpio sin consumir recursos extra."
        }
      ]
    },
    benchmarks: {
      tag: "RENDIMIENTO COMPARATIVO",
      title: "Benchmarks Reales vs La Competencia",
      subtitle: "Pruebas acústicas realizadas en entornos reales con ruido de teclado mecánico azul, ventiladores de alto flujo y tráfico urbano.",
      tableHeaders: {
        feature: "Métrica / Característica",
        voiceclear: "Voice Clear AI",
        krisp: "Krisp AI",
        rtx: "NVIDIA RTX Voice",
        discord: "Discord Krisp Integrado"
      },
      rows: [
        {
          metric: "Latencia de Procesamiento",
          vc: "9.8 ms (Instantáneo)",
          krisp: "35 - 55 ms",
          rtx: "25 - 40 ms",
          discord: "45 - 60 ms"
        },
        {
          metric: "Requisito de GPU",
          vc: "0% (Solo CPU)",
          krisp: "0% (CPU intensivo)",
          rtx: "Requiere GPU RTX/GTX",
          discord: "0% (Solo en app)"
        },
        {
          metric: "Reducción Acústica Máxima",
          vc: "> 51.5 dB",
          krisp: "~45 dB",
          rtx: "~48 dB",
          discord: "~38 dB"
        },
        {
          metric: "Compatibilidad del Sistema",
          vc: "Universal (Driver WDK)",
          krisp: "Universal",
          rtx: "Solo PCs con GPU NVIDIA",
          discord: "Solo dentro de Discord"
        },
        {
          metric: "Privacidad y Procesamiento",
          vc: "100% On-Device",
          krisp: "Local / Telemetría",
          rtx: "Local",
          discord: "Servidores Discord"
        },
        {
          metric: "Modelo de Precio",
          vc: "Desde $6.99 (Pago Único)",
          krisp: "$8 - $12/mes recurrente",
          rtx: "Gratis pero GPU costosa",
          discord: "Limitado a la app"
        }
      ]
    },
    compatibility: {
      tag: "ECOSISTEMA",
      title: "Funciona con todas tus aplicaciones favoritas",
      subtitle: "Solo selecciona 'Voice Clear Virtual Mic' en la configuración de audio de tu software.",
      apps: [
        { name: "Boostlingo", category: "Interpretación & Teletrabajo" },
        { name: "Discord", category: "Gaming & Comunidades" },
        { name: "OBS Studio", category: "Streaming & Grabación" },
        { name: "Zoom", category: "Reuniones de Negocios" },
        { name: "Microsoft Teams", category: "Colaboración Empresarial" },
        { name: "Twitch Studio", category: "Transmisiones en Vivo" },
        { name: "Slack", category: "Audio Huddles" },
        { name: "Audacity / DAWs", category: "Producción de Podcasts" }
      ]
    },
    howItWorks: {
      tag: "CONFIGURACIÓN SENCILLA",
      title: "Listo en 3 sencillos pasos",
      subtitle: "Empieza a sonar como un profesional de estudio en menos de 2 minutos.",
      steps: [
        {
          step: "01",
          title: "Descarga e Instala",
          desc: "Ejecuta el instalador en Windows. Instalará automáticamente el driver virtual de audio seguro y la aplicación."
        },
        {
          step: "02",
          title: "Selecciona tu Micrófono Físico",
          desc: "Abre Voice Clear AI y elige tu micrófono USB, headset o interfaz de audio en el menú desplegable."
        },
        {
          step: "03",
          title: "Elige Voice Clear en tus Apps",
          desc: "En Boostlingo, Discord, Zoom, OBS o Teams, configura tu dispositivo de entrada como 'Voice Clear Virtual Mic'. ¡Listo!"
        }
      ]
    },
    pricing: {
      tag: "PLANES Y LICENCIAS",
      title: "Precios accesibles, pago único de por vida",
      subtitle: "Sin suscripciones mensuales recurrentes. Adquiere tu licencia definitiva con atención directa por WhatsApp.",
      billedOnce: "Pago único de por vida",
      popularTag: "MEJOR VALOR",
      plans: [
        {
          id: "single",
          name: "Licencia 1 Dispositivo",
          price: "$6.99",
          oldPrice: "$15",
          period: "Pago único de por vida",
          desc: "La solución perfecta para tu PC o Laptop de trabajo y estudio personal.",
          buttonText: "Comprar por WhatsApp ($6.99)",
          whatsappMsg: "Hola, quiero comprar la licencia de Voice Clear AI (1 Dispositivo - $6.99 USD).",
          features: [
            "Licencia vitalicia para 1 PC (Windows 10 / 11)",
            "Supresión extrema DeepFilterNet3 (-51.5 dB)",
            "1 Año de Soporte Técnico Directo",
            "Garantía de reembolso de 5 días",
            "Latencia ultra-baja (<10ms) en CPU AVX2",
            "Compatible con Boostlingo, Discord, Zoom, Teams, OBS",
            "Actualizaciones de por vida incluidas"
          ],
          featured: false
        },
        {
          id: "combo",
          name: "Combo 2 Dispositivos",
          price: "$10",
          oldPrice: "$25",
          period: "Pago único de por vida",
          desc: "Ahorra al máximo equipando tu PC de escritorio y tu Laptop personal o familiar.",
          buttonText: "Comprar Combo por WhatsApp ($10)",
          whatsappMsg: "Hola, quiero comprar el Combo de Voice Clear AI (2 Dispositivos - $10 USD).",
          features: [
            "Licencia vitalicia para 2 PCs (Windows 10 / 11)",
            "Supresión extrema DeepFilterNet3 (-51.5 dB)",
            "1 Año de Soporte Técnico Prioritario",
            "Garantía de reembolso de 5 días",
            "Latencia ultra-baja (<10ms) en CPU AVX2",
            "Compatible con Boostlingo, Discord, Zoom, Teams, OBS",
            "Actualizaciones de por vida incluidas"
          ],
          featured: true
        }
      ],
      moneyBack: "Garantía de reembolso de 5 días y 1 año de soporte técnico incluido."
    },
    faq: {
      tag: "PREGUNTAS FRECUENTES",
      title: "Resolvemos todas tus dudas",
      items: [
        {
          q: "¿Cómo es el proceso de compra por WhatsApp?",
          a: "Al hacer clic en el botón de compra, te pondrás en contacto directo con nosotros por WhatsApp (+505 8741 4791). Te facilitamos los métodos de pago disponibles y te entregamos inmediatamente tu clave de activación junto al instalador completo."
        },
        {
          q: "¿Qué incluye la garantía de 5 días?",
          a: "Si durante los primeros 5 días el software no cumple con tus expectativas o no es compatible con tu equipo, te devolvemos el 100% de tu dinero sin complicaciones."
        },
        {
          q: "¿Qué incluye el año de soporte técnico?",
          a: "Tendrás asistencia personalizada directa por WhatsApp y correo electrónico para ayudarte en la instalación, configuración del micrófono virtual y calibración del audio durante 1 año completo."
        },
        {
          q: "¿Requiere conexión a internet para funcionar?",
          a: "No. Voice Clear AI procesa el 100% del audio de manera local en tu procesador (CPU) utilizando el motor ONNX Runtime con aceleración AVX2. No requiere internet y tus audios nunca se transmiten."
        },
        {
          q: "¿Es compatible con Boostlingo, Zoom, Discord y Teams?",
          a: "Sí. Voice Clear AI crea un dispositivo de audio virtual llamado 'Voice Clear Virtual Mic' en Windows a través del driver AVStream. Puedes seleccionarlo en cualquier programa que admita un micrófono."
        },
        {
          q: "¿Necesito una tarjeta gráfica dedicada potente como NVIDIA RTX?",
          a: "No. A diferencia de soluciones que consumen tu tarjeta de video, Voice Clear AI está hiper-optimizado para la CPU (Intel 8va generación en adelante o AMD Ryzen equivalente). Tu GPU queda libre."
        }
      ]
    },
    ctaBanner: {
      title: "¿Listo para hablar con la máxima claridad?",
      subtitle: "Consigue tu licencia definitiva por solo $6.99 USD o el combo de 2 dispositivos por $10 USD con soporte técnico incluido.",
      buttonDownload: "Descargar Prueba Gratis",
      buttonPro: "Comprar por WhatsApp ($6.99 USD)"
    },
    footer: {
      tagline: "Cancelación de ruido en tiempo real impulsada por Inteligencia Artificial acústica para Windows.",
      product: "Producto",
      features: "Características",
      demo: "Demo A/B",
      pricing: "Precios",
      downloads: "Descargas",
      contactTitle: "Contacto & Soporte",
      emailLabel: "Correo Electrónico:",
      emailValue: "bpalacios347@gmail.com",
      whatsappLabel: "WhatsApp:",
      whatsappValue: "+505 8741 4791",
      legal: "Garantía & Soporte",
      warranty: "Garantía de reembolso de 5 días",
      support: "1 Año de Soporte Técnico",
      license: "Licencia de Software",
      copyright: "© 2026 Voice Clear AI. Todos los derechos reservados. Contacto: bpalacios347@gmail.com"
    },
    modals: {
      download: {
        title: "Descargar Voice Clear AI",
        subtitle: "Versión v1.0.0 para Windows 10 / 11 (64-bit)",
        btnDownload: "Iniciar Descarga (.EXE)",
        fileInfo: "Tamaño: ~45 MB • Firma Digital WDK • Sin publicidad ni telemetría",
        stepsTitle: "Instrucciones de Instalación:",
        step1: "1. Ejecuta el archivo instalador 'VoiceClearAI_Setup.exe'.",
        step2: "2. Acepta la instalación del driver virtual seguro.",
        step3: "3. Abre la aplicación, selecciona tu micrófono y disfruta de tu voz cristalina."
      }
    }
  },
  en: {
    nav: {
      features: "Features",
      demo: "A/B Demo",
      simulator: "App Simulator",
      benchmarks: "Benchmarks",
      pricing: "Pricing",
      faq: "FAQ",
      downloadFree: "Download Free",
      buyPro: "Buy via WhatsApp ($6.99)",
      contact: "Contact"
    },
    hero: {
      badge: "Voice Clear AI v1.0 — DeepFilterNet3 CPU Engine",
      titleStart: "Your Voice.",
      titleHighlight: "Zero Noise.",
      titleEnd: "100% Private On-Device.",
      subtitle: "Eliminate mechanical keyboard clatter, fan whirrs, room echoes, and background noise in real-time with AI. Sub-10ms latency, 0% GPU usage, and zero audio data sent to the cloud.",
      ctaDownload: "Download Free Trial",
      ctaDownloadSub: "Windows 10 / 11 (64-bit)",
      ctaPricing: "Get License ($6.99)",
      stats: [
        { label: "Noise Reduction", value: "-51.5 dB" },
        { label: "Real-Time Latency", value: "9.8 ms" },
        { label: "Audio Fidelity", value: "48 kHz HD" },
        { label: "Processing", value: "100% On-Device" }
      ]
    },
    comparator: {
      tag: "HEAR THE DIFFERENCE",
      title: "Listen to AI Noise Suppression in Real-Time",
      subtitle: "Click play and switch seamlessly between the original noisy audio (mechanical keyboard, office chatter) and Voice Clear AI enhanced audio.",
      playing: "Playing",
      paused: "Paused",
      modeNoisy: "Original Audio (Noisy)",
      modeClean: "Voice Clear AI (Clean)",
      clickToToggle: "Switch live to compare",
      sliderHint: "Drag seeker or click anywhere along timeline",
      specsBanner: {
        attenuation: "51.58 dB Acoustic Suppression",
        engine: "ONNX Runtime + AVX2 oneDNN",
        latency: "9.84ms Processing Duration"
      }
    },
    simulator: {
      tag: "USER EXPERIENCE",
      title: "Clean, Modern Interface. Set & Forget.",
      subtitle: "Crafted with modern Qt 6 and connected to a background Windows Service. Configure your mic once and let it run seamlessly.",
      statusActive: "ACTIVE PROTECTION",
      statusInactive: "PROTECTION PAUSED",
      toggleHint: "Click the microphone button to toggle the filter",
      micSelect: "Hardware Physical Microphone",
      virtualDevice: "Virtual Output Device",
      virtualDeviceName: "Voice Clear Virtual Mic (AVStream WDK)",
      noiseSuppressionLevel: "AI Suppression Intensity",
      cpuUsage: "CPU Usage",
      latencyText: "Estimated Latency",
      runInBackground: "Run silently as background Windows Service on startup",
      autoStartActive: "Active in background"
    },
    features: {
      tag: "WHY VOICE CLEAR AI",
      title: "Engineered for Peak Clarity & Zero Distractions",
      subtitle: "The ultimate synergy between state-of-the-art acoustic AI and native Windows performance optimization.",
      items: [
        {
          icon: "ShieldCheck",
          title: "100% On-Device & Privacy-First",
          desc: "Your conversations never leave your computer. DeepFilterNet3 runs entirely on your local CPU, meeting stringent enterprise and medical compliance standards."
        },
        {
          icon: "Zap",
          title: "Ultra-Low Latency (<10ms)",
          desc: "Featuring a high-performance C++ pipeline and lock-free SPSC ring buffers, your voice streams to Boostlingo, Discord, Twitch, or Zoom without perceptible delay."
        },
        {
          icon: "Cpu",
          title: "0% GPU Required (AVX2 / oneDNN)",
          desc: "No expensive RTX graphics card needed. Purpose-built for Intel Core (8th Gen+) and AMD Ryzen CPUs with SIMD vectorized instruction sets."
        },
        {
          icon: "Mic",
          title: "Native Virtual Microphone Driver",
          desc: "Ships with a certified WDK AVStream Windows virtual audio device. Any app expecting a microphone will instantly recognize 'Voice Clear Virtual Mic'."
        },
        {
          icon: "Sparkles",
          title: "48 kHz Studio Fidelity",
          desc: "Unlike standard filters that muffle or distort vocal tones, Voice Clear AI preserves the natural warmth, crisp consonants, and depth of your voice."
        },
        {
          icon: "Layers",
          title: "Silent Background Service",
          desc: "Architectural separation between Qt 6 GUI and real-time audio service. Close the window anytime; audio filtering stays active without wasting CPU cycles."
        }
      ]
    },
    benchmarks: {
      tag: "COMPARATIVE PERFORMANCE",
      title: "Real Benchmarks vs Industry Competitors",
      subtitle: "Acoustic stress tests recorded in real-world environments featuring clicky mechanical switches, heavy industrial fans, and road traffic.",
      tableHeaders: {
        feature: "Metric / Feature",
        voiceclear: "Voice Clear AI",
        krisp: "Krisp AI",
        rtx: "NVIDIA RTX Voice",
        discord: "Discord Built-in Krisp"
      },
      rows: [
        {
          metric: "Processing Latency",
          vc: "9.8 ms (Instant)",
          krisp: "35 - 55 ms",
          rtx: "25 - 40 ms",
          discord: "45 - 60 ms"
        },
        {
          metric: "GPU Requirement",
          vc: "0% (CPU only)",
          krisp: "0% (Heavy CPU load)",
          rtx: "Requires RTX/GTX GPU",
          discord: "0% (In-app only)"
        },
        {
          metric: "Maximum Noise Attenuation",
          vc: "> 51.5 dB",
          krisp: "~45 dB",
          rtx: "~48 dB",
          discord: "~38 dB"
        },
        {
          metric: "System-wide Compatibility",
          vc: "Universal (WDK Driver)",
          krisp: "Universal",
          rtx: "NVIDIA PCs only",
          discord: "Discord app only"
        },
        {
          metric: "Privacy & Processing",
          vc: "100% On-Device",
          krisp: "Local / Telemetry",
          rtx: "Local",
          discord: "Discord Cloud Servers"
        },
        {
          metric: "Pricing Model",
          vc: "From $6.99 (One-Time)",
          krisp: "$8 - $12/mo recurring",
          rtx: "Free but expensive GPU",
          discord: "App restricted"
        }
      ]
    },
    compatibility: {
      tag: "ECOSYSTEM",
      title: "Seamlessly Compatible with All Your Apps",
      subtitle: "Just select 'Voice Clear Virtual Mic' in the audio preferences of your favorite software.",
      apps: [
        { name: "Boostlingo", category: "Interpreting & Remote Work" },
        { name: "Discord", category: "Gaming & Communities" },
        { name: "OBS Studio", category: "Streaming & Recording" },
        { name: "Zoom", category: "Executive Meetings" },
        { name: "Microsoft Teams", category: "Enterprise Collaboration" },
        { name: "Twitch Studio", category: "Live Broadcasts" },
        { name: "Slack", category: "Team Huddles" },
        { name: "Audacity / DAWs", category: "Podcast Production" }
      ]
    },
    howItWorks: {
      tag: "EASY SETUP",
      title: "Ready in 3 Simple Steps",
      subtitle: "Sound like a broadcast studio professional in under two minutes.",
      steps: [
        {
          step: "01",
          title: "Download & Install",
          desc: "Run the Windows installer. It installs the certified virtual audio driver and the lightweight app automatically."
        },
        {
          step: "02",
          title: "Select Physical Microphone",
          desc: "Launch Voice Clear AI and choose your USB microphone, headset, or audio interface from the dropdown list."
        },
        {
          step: "03",
          title: "Choose Voice Clear in Your Apps",
          desc: "Set your input device to 'Voice Clear Virtual Mic' in Boostlingo, Discord, Zoom, OBS, or Teams. You're done!"
        }
      ]
    },
    pricing: {
      tag: "PLANS & LICENSES",
      title: "Affordable One-Time Pricing",
      subtitle: "No expensive recurring monthly subscriptions. Secure your lifetime license with direct WhatsApp support.",
      billedOnce: "One-time lifetime purchase",
      popularTag: "BEST VALUE",
      plans: [
        {
          id: "single",
          name: "1 Device License",
          price: "$6.99",
          oldPrice: "$15",
          period: "One-time lifetime payment",
          desc: "The perfect solution for your personal work or study PC/Laptop.",
          buttonText: "Buy via WhatsApp ($6.99)",
          whatsappMsg: "Hello, I want to purchase the Voice Clear AI license (1 Device - $6.99 USD).",
          features: [
            "Lifetime license for 1 PC (Windows 10 / 11)",
            "Extreme DeepFilterNet3 suppression (-51.5 dB)",
            "1 Year Direct Technical Support",
            "5-Day Money-Back Guarantee",
            "Ultra-low latency (<10ms) AVX2 profile",
            "Compatible with Boostlingo, Discord, Zoom, Teams, OBS",
            "Lifetime updates included"
          ],
          featured: false
        },
        {
          id: "combo",
          name: "2 Devices Combo",
          price: "$10",
          oldPrice: "$25",
          period: "One-time lifetime payment",
          desc: "Save big by covering your desktop workstation and personal laptop.",
          buttonText: "Buy Combo via WhatsApp ($10)",
          whatsappMsg: "Hello, I want to purchase the Voice Clear AI Combo (2 Devices - $10 USD).",
          features: [
            "Lifetime license for 2 PCs (Windows 10 / 11)",
            "Extreme DeepFilterNet3 suppression (-51.5 dB)",
            "1 Year Priority Technical Support",
            "5-Day Money-Back Guarantee",
            "Ultra-low latency (<10ms) AVX2 profile",
            "Compatible with Boostlingo, Discord, Zoom, Teams, OBS",
            "Lifetime updates included"
          ],
          featured: true
        }
      ],
      moneyBack: "5-day money-back guarantee and 1-year technical support included."
    },
    faq: {
      tag: "FREQUENTLY ASKED QUESTIONS",
      title: "Got Questions? We Have Answers.",
      items: [
        {
          q: "How does purchasing via WhatsApp work?",
          a: "Clicking the buy button connects you directly to us on WhatsApp (+505 8741 4791). We provide available payment methods and immediately send your activation license key and full installer."
        },
        {
          q: "What does the 5-day money-back guarantee cover?",
          a: "If during the first 5 days the software does not meet your expectations or isn't compatible with your setup, we issue a 100% full refund with no hassle."
        },
        {
          q: "What does the 1-year technical support include?",
          a: "You get direct personal assistance via WhatsApp and email to help you with installation, virtual mic configuration, and audio fine-tuning for a full year."
        },
        {
          q: "Does it require an active internet connection?",
          a: "No. Voice Clear AI processes 100% of your audio locally on your CPU using ONNX Runtime with AVX2 acceleration. It runs completely offline with zero telemetry or audio uploads."
        },
        {
          q: "Is it compatible with Boostlingo, Zoom, Discord, and Teams?",
          a: "Yes. Voice Clear AI provides a Windows virtual audio device ('Voice Clear Virtual Mic') via its AVStream driver. You can select it in any software that accepts microphone input."
        },
        {
          q: "Do I need a dedicated GPU like NVIDIA RTX?",
          a: "No. Unlike other tools that consume your GPU, Voice Clear AI is engineered purely for CPUs (Intel 8th Gen+ or AMD Ryzen). Your graphics card remains 100% free."
        }
      ]
    },
    ctaBanner: {
      title: "Ready to Speak with Crystal-Clear Confidence?",
      subtitle: "Get your lifetime license for only $6.99 USD or the 2-device combo for $10 USD with full technical support included.",
      buttonDownload: "Download Free Trial",
      buttonPro: "Buy via WhatsApp ($6.99 USD)"
    },
    footer: {
      tagline: "Real-time AI acoustic noise suppression for Windows.",
      product: "Product",
      features: "Features",
      demo: "A/B Demo",
      pricing: "Pricing",
      downloads: "Downloads",
      contactTitle: "Contact & Support",
      emailLabel: "Email:",
      emailValue: "bpalacios347@gmail.com",
      whatsappLabel: "WhatsApp:",
      whatsappValue: "+505 8741 4791",
      legal: "Warranty & Support",
      warranty: "5-day money-back guarantee",
      support: "1 Year Technical Support",
      license: "Software License",
      copyright: "© 2026 Voice Clear AI. All rights reserved. Contact: bpalacios347@gmail.com"
    },
    modals: {
      download: {
        title: "Download Voice Clear AI",
        subtitle: "Version v1.0.0 for Windows 10 / 11 (64-bit)",
        btnDownload: "Start Download (.EXE)",
        fileInfo: "Size: ~45 MB • Signed WDK Driver • Ad-free & Telemetry-free",
        stepsTitle: "Quick Installation Steps:",
        step1: "1. Run the installer 'VoiceClearAI_Setup.exe'.",
        step2: "2. Accept the secure virtual audio driver setup.",
        step3: "3. Launch Voice Clear AI, select your physical mic, and enjoy clean audio."
      }
    }
  }
};
