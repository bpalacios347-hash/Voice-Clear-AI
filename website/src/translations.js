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
      buyPro: "Comprar Pro"
    },
    hero: {
      badge: "Voice Clear AI v1.0 — Motor DeepFilterNet3 en CPU",
      titleStart: "Tu Voz.",
      titleHighlight: "Cero Ruido.",
      titleEnd: "100% Privado en tu PC.",
      subtitle: "Elimina teclados mecánicos, ventiladores, ecos y ruidos de fondo en tiempo real con IA. Menos de 10ms de latencia, 0% de uso de GPU y sin enviar un solo byte de audio a la nube.",
      ctaDownload: "Descargar para Windows",
      ctaDownloadSub: "Windows 10 / 11 (64-bit) • Gratis",
      ctaPricing: "Ver Planes Pro",
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
          desc: "Con un pipeline optimizado en C++ y buffers lock-free SPSC de baja latencia, tu voz llega a Discord, Twitch o Zoom sin ningún desfase o eco perceptible."
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
          vc: "Pago Único / Accesible",
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
        { name: "Discord", category: "Gaming & Comunidades" },
        { name: "OBS Studio", category: "Streaming & Grabación" },
        { name: "Zoom", category: "Reuniones de Negocios" },
        { name: "Google Meet", category: "Videollamadas" },
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
          desc: "En Discord, Zoom, OBS o Teams, configura tu dispositivo de entrada como 'Voice Clear Virtual Mic'. ¡Listo!"
        }
      ]
    },
    pricing: {
      tag: "PLANES Y LICENCIAS",
      title: "Precios claros y sin sorpresas",
      subtitle: "Olvídate de costosas suscripciones mensuales. Consigue tu licencia definitiva para Windows.",
      billedOnce: "Pago único de por vida",
      popularTag: "MÁS POPULAR",
      plans: [
        {
          id: "free",
          name: "Starter Free",
          price: "$0",
          period: "Para siempre",
          desc: "Ideal para probar el poder de la cancelación de audio en tu setup personal.",
          buttonText: "Descargar Gratis",
          features: [
            "Cancelación de ruido básica en tiempo real",
            "Soporte para 1 micrófono físico",
            "Audio a 48 kHz",
            "Driver virtual para Windows",
            "Soporte de la comunidad"
          ],
          featured: false
        },
        {
          id: "pro",
          name: "Pro Lifetime",
          price: "$29",
          oldPrice: "$59",
          period: "Pago único para siempre",
          desc: "La mejor opción para streamers, profesionales remotos, gamers y podcasters.",
          buttonText: "Obtener Licencia Pro",
          features: [
            "Supresión extrema DeepFilterNet3 (-51.5 dB)",
            "Latencia ultra-baja (<10ms) perfil AVX2",
            "Todas las actualizaciones futuras v1.x y v2.x",
            "Licencia vitalicia para hasta 2 PCs personales",
            "Eliminación de teclado mecánico, tráfico y ventiladores",
            "Soporte prioritario por email"
          ],
          featured: true
        },
        {
          id: "studio",
          name: "Studio & Commercial",
          price: "$69",
          oldPrice: "$120",
          period: "Pago único comercial",
          desc: "Para estudios de producción, agencias de contenido y creadores con múltiples equipos.",
          buttonText: "Comprar Licencia Studio",
          features: [
            "Todo lo incluido en el plan Pro",
            "Licencia para uso comercial / monetización",
            "Instalación en hasta 5 dispositivos",
            "Curvas de ecualización y presets de voz pro",
            "Soporte VIP por Discord y asistencia técnica directa",
            "Acceso anticipado a nuevos modelos de IA"
          ],
          featured: false
        }
      ],
      moneyBack: "Garantía de reembolso de 30 días sin preguntas."
    },
    testimonials: {
      tag: "TESTIMONIOS",
      title: "Lo que dicen quienes ya no tienen ruido de fondo",
      items: [
        {
          name: "Carlos Mendoza",
          role: "Streamer en Twitch & Creador de Contenido",
          avatar: "https://images.unsplash.com/photo-1534528741775-53994a69daeb?w=150&auto=format&fit=crop&q=80",
          comment: "Uso un teclado mecánico con switches azules muy ruidosos y el ventilador en verano al máximo. Voice Clear AI eliminó todo por completo sin tocar mi tarjeta gráfica ni bajar FPS en mis juegos."
        },
        {
          name: "Elena Rostova",
          role: "Senior Engineering Manager (Trabajo Remoto)",
          avatar: "https://images.unsplash.com/photo-1580489944761-15a19d654956?w=150&auto=format&fit=crop&q=80",
          comment: "Tengo reuniones todo el día con directivos. Mis dos perros ladran frecuentemente y con Voice Clear AI nadie en mis llamadas de Teams o Zoom escucha nada más que mi voz limpia. Imprescindible."
        },
        {
          name: "Marcos Del Valle",
          role: "Host de Podcast 'Tech Pulse'",
          avatar: "https://images.unsplash.com/photo-1507003211169-0a1dd7228f2d?w=150&auto=format&fit=crop&q=80",
          comment: "La calidad a 48kHz es asombrosa. Otros supresores hacen que tu voz suene como debajo del agua o robotizada. Voice Clear AI mantiene el cuerpo natural y los graves cálidos de mi voz."
        }
      ]
    },
    faq: {
      tag: "PREGUNTAS FRECUENTES",
      title: "Resolvemos todas tus dudas",
      items: [
        {
          q: "¿Requiere conexión a internet para funcionar?",
          a: "No. Voice Clear AI procesa el 100% del audio de manera local en tu procesador (CPU) utilizando el motor ONNX Runtime con aceleración AVX2. No requiere internet y tus audios nunca se transmiten."
        },
        {
          q: "¿Necesito una tarjeta gráfica dedicada potente como NVIDIA RTX?",
          a: "No. A diferencia de soluciones que consumen un 15-25% de tu tarjeta de video, Voice Clear AI está hiper-optimizado para la CPU (Intel 8va generación en adelante o AMD Ryzen equivalente). Tu GPU queda 100% libre para juegos o renderizado."
        },
        {
          q: "¿Es compatible con Discord, Zoom, OBS y Google Meet?",
          a: "Sí. Voice Clear AI crea un dispositivo de audio virtual llamado 'Voice Clear Virtual Mic' en Windows a través del driver AVStream. Puedes seleccionarlo en cualquier programa que admita un micrófono."
        },
        {
          q: "¿Qué nivel de latencia introduce?",
          a: "Nuestras mediciones p50 registran solo 9.84 milisegundos de tiempo de procesamiento por bloque de 480 muestras a 48kHz, lo cual es totalmente imperceptible para el oído humano y no causa desfase con la cámara web."
        },
        {
          q: "¿Cómo recibo mi clave de licencia tras la compra?",
          a: "Inmediatamente después de completar tu pago seguro, recibirás un correo con tu clave de activación de por vida y el enlace directo de descarga del instalador Pro."
        },
        {
          q: "¿Qué sistemas operativos son compatibles?",
          a: "Actualmente es compatible de forma nativa con Windows 10 (64-bit) y Windows 11 (64-bit)."
        }
      ]
    },
    ctaBanner: {
      title: "¿Listo para hablar con la máxima claridad?",
      subtitle: "Únete a miles de streamers, profesionales y creadores que ya disfrutan de un audio cristalino sin ruidos molestos.",
      buttonDownload: "Descargar Gratis para Windows",
      buttonPro: "Comprar Licencia Pro ($29 USD)"
    },
    footer: {
      tagline: "Cancelación de ruido en tiempo real impulsada por Inteligencia Artificial acústica para Windows.",
      product: "Producto",
      features: "Características",
      demo: "Demo A/B",
      pricing: "Precios",
      downloads: "Descargas",
      resources: "Recursos",
      docs: "Documentación",
      github: "Repositorio GitHub",
      benchmarks: "Reporte de Benchmarks",
      releaseNotes: "Notas de la Versión",
      legal: "Legal",
      privacy: "Privacidad",
      terms: "Términos de Servicio",
      license: "Licencia de Software",
      copyright: "© 2026 Voice Clear AI. Todos los derechos reservados. Desarrollado con DeepFilterNet3 y WDK."
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
      },
      checkout: {
        title: "Adquirir Licencia",
        subtitle: "Pago seguro único de por vida con garantía de 30 días",
        planLabel: "Plan Seleccionado",
        emailPlaceholder: "tu-correo@ejemplo.com",
        cardPlaceholder: "Número de tarjeta",
        expiryPlaceholder: "MM/AA",
        cvcPlaceholder: "CVC",
        payButton: "Pagar y Obtener Clave de Activación",
        secureNote: "🔒 Encriptación SSL de 256 bits. Aceptamos Tarjetas de Crédito, Débito y PayPal."
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
      buyPro: "Buy Pro"
    },
    hero: {
      badge: "Voice Clear AI v1.0 — DeepFilterNet3 CPU Engine",
      titleStart: "Your Voice.",
      titleHighlight: "Zero Noise.",
      titleEnd: "100% Private On-Device.",
      subtitle: "Eliminate mechanical keyboard clatter, fan whirrs, room echoes, and background noise in real-time with AI. Sub-10ms latency, 0% GPU usage, and zero audio data sent to the cloud.",
      ctaDownload: "Download for Windows",
      ctaDownloadSub: "Windows 10 / 11 (64-bit) • Free",
      ctaPricing: "View Pro Plans",
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
          desc: "Featuring a high-performance C++ pipeline and lock-free SPSC ring buffers, your voice streams to Discord, Twitch, or Zoom without perceptible delay."
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
          vc: "One-Time / Affordable",
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
        { name: "Discord", category: "Gaming & Communities" },
        { name: "OBS Studio", category: "Streaming & Recording" },
        { name: "Zoom", category: "Executive Meetings" },
        { name: "Google Meet", category: "Video Calls" },
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
          desc: "Set your input device to 'Voice Clear Virtual Mic' in Discord, Zoom, OBS, or Teams. You're done!"
        }
      ]
    },
    pricing: {
      tag: "PLANS & LICENSES",
      title: "Transparent, One-Time Pricing",
      subtitle: "Say goodbye to costly monthly subscriptions. Secure your lifetime Windows license.",
      billedOnce: "One-time lifetime purchase",
      popularTag: "MOST POPULAR",
      plans: [
        {
          id: "free",
          name: "Starter Free",
          price: "$0",
          period: "Forever",
          desc: "Ideal for testing the noise cancellation capabilities on your home setup.",
          buttonText: "Download Free",
          features: [
            "Basic real-time noise suppression",
            "Support for 1 physical microphone",
            "48 kHz high fidelity audio",
            "Windows virtual driver included",
            "Community support"
          ],
          featured: false
        },
        {
          id: "pro",
          name: "Pro Lifetime",
          price: "$29",
          oldPrice: "$59",
          period: "One-time lifetime payment",
          desc: "The ultimate choice for streamers, remote workers, gamers, and podcasters.",
          buttonText: "Get Pro License",
          features: [
            "Extreme DeepFilterNet3 suppression (-51.5 dB)",
            "Ultra-low latency (<10ms) AVX2 profile",
            "All future v1.x & v2.x updates included",
            "Lifetime license for up to 2 personal PCs",
            "Eliminates mechanical keyboards, fans, and traffic",
            "Priority email support"
          ],
          featured: true
        },
        {
          id: "studio",
          name: "Studio & Commercial",
          price: "$69",
          oldPrice: "$120",
          period: "One-time commercial license",
          desc: "For production studios, content agencies, and creators with multiple workstations.",
          buttonText: "Get Studio License",
          features: [
            "Everything included in Pro",
            "Commercial monetization license",
            "Install on up to 5 devices",
            "Professional vocal EQ curves and presets",
            "VIP Discord support & direct technical assistance",
            "Early access to next-gen AI models"
          ],
          featured: false
        }
      ],
      moneyBack: "30-day no-questions-asked money-back guarantee."
    },
    testimonials: {
      tag: "TESTIMONIALS",
      title: "Loved by Creators, Streamers & Remote Pros",
      items: [
        {
          name: "Carlos Mendoza",
          role: "Twitch Streamer & Content Creator",
          avatar: "https://images.unsplash.com/photo-1534528741775-53994a69daeb?w=150&auto=format&fit=crop&q=80",
          comment: "I use loud blue mechanical switches and have a high-speed room fan. Voice Clear AI cancels it all out without touching my GPU or dropping a single frame in games."
        },
        {
          name: "Elena Rostova",
          role: "Senior Engineering Manager (Remote)",
          avatar: "https://images.unsplash.com/photo-1580489944761-15a19d654956?w=150&auto=format&fit=crop&q=80",
          comment: "I'm in executive meetings all day. My two dogs bark frequently, and with Voice Clear AI, no one on Teams or Zoom hears anything except my voice. Essential tool."
        },
        {
          name: "Marcos Del Valle",
          role: "Host of 'Tech Pulse' Podcast",
          avatar: "https://images.unsplash.com/photo-1507003211169-0a1dd7228f2d?w=150&auto=format&fit=crop&q=80",
          comment: "The 48kHz studio fidelity is incredible. Other noise reducers make you sound underwater or robotic. Voice Clear AI preserves the warm low end and crisp high end of my mic."
        }
      ]
    },
    faq: {
      tag: "FREQUENTLY ASKED QUESTIONS",
      title: "Got Questions? We Have Answers.",
      items: [
        {
          q: "Does it require an active internet connection?",
          a: "No. Voice Clear AI processes 100% of your audio locally on your CPU using ONNX Runtime with AVX2 acceleration. It runs completely offline with zero telemetry or audio uploads."
        },
        {
          q: "Do I need a dedicated GPU like NVIDIA RTX?",
          a: "No. Unlike other tools that consume 15-25% of your GPU, Voice Clear AI is engineered purely for CPUs (Intel 8th Gen+ or AMD Ryzen). Your graphics card remains 100% available for gaming and rendering."
        },
        {
          q: "Is it compatible with Discord, Zoom, OBS, and Google Meet?",
          a: "Yes. Voice Clear AI provides a Windows virtual audio device ('Voice Clear Virtual Mic') via its AVStream driver. You can select it in any software that accepts microphone input."
        },
        {
          q: "How much latency does it introduce?",
          a: "Our p50 benchmarks measure only 9.84 milliseconds of processing duration for 480-sample blocks at 48kHz, which is imperceptible to human ears and keeps perfect sync with your webcam."
        },
        {
          q: "How do I receive my license key after purchase?",
          a: "Immediately upon checkout, you will receive an email containing your lifetime license key and the direct download link for the Pro installer."
        },
        {
          q: "Which operating systems are supported?",
          a: "Voice Clear AI natively supports Windows 10 (64-bit) and Windows 11 (64-bit)."
        }
      ]
    },
    ctaBanner: {
      title: "Ready to Speak with Crystal-Clear Confidence?",
      subtitle: "Join thousands of streamers, developers, and creators who eliminated background noise forever.",
      buttonDownload: "Download Free for Windows",
      buttonPro: "Get Pro License ($29 USD)"
    },
    footer: {
      tagline: "Real-time AI acoustic noise suppression for Windows.",
      product: "Product",
      features: "Features",
      demo: "A/B Demo",
      pricing: "Pricing",
      downloads: "Downloads",
      resources: "Resources",
      docs: "Documentation",
      github: "GitHub Repository",
      benchmarks: "Benchmark Report",
      releaseNotes: "Release Notes",
      legal: "Legal",
      privacy: "Privacy Policy",
      terms: "Terms of Service",
      license: "Software License",
      copyright: "© 2026 Voice Clear AI. All rights reserved. Powered by DeepFilterNet3 & WDK."
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
      },
      checkout: {
        title: "Get License",
        subtitle: "Secure one-time lifetime payment with 30-day money back guarantee",
        planLabel: "Selected Plan",
        emailPlaceholder: "your-email@example.com",
        cardPlaceholder: "Card number",
        expiryPlaceholder: "MM/YY",
        cvcPlaceholder: "CVC",
        payButton: "Complete Purchase & Get Key",
        secureNote: "🔒 256-bit SSL Encryption. We accept Credit Cards, Debit, and PayPal."
      }
    }
  }
};
