import { Search, Download, Check, Star, Filter } from "lucide-react";

interface Mod {
  id: string;
  name: string;
  author: string;
  description: string;
  downloads: string;
  rating: number;
  category: string;
  installed: boolean;
  icon: string;
}

const mods: Mod[] = [
  {
    id: "1",
    name: "OptiFine",
    author: "sp614x",
    description: "Оптимизация графики, поддержка шейдеров, HD текстур и другие улучшения",
    downloads: "2.1M",
    rating: 4.9,
    category: "Оптимизация",
    installed: true,
    icon: "🔧",
  },
  {
    id: "2",
    name: "Sodium",
    author: "CaffeineMC",
    description: "Мощная оптимизация рендеринга для значительного увеличения FPS",
    downloads: "1.8M",
    rating: 4.8,
    category: "Оптимизация",
    installed: true,
    icon: "⚡",
  },
  {
    id: "3",
    name: "Litematica",
    author: "masa",
    description: "Сохраняй и загружай схематики построек прямо в игре",
    downloads: "890K",
    rating: 4.7,
    category: "Утилиты",
    installed: false,
    icon: "📐",
  },
  {
    id: "4",
    name: "Iris Shaders",
    author: "IrisShaders",
    description: "Шейдеры для Fabric с совместимостью с Sodium",
    downloads: "1.2M",
    rating: 4.8,
    category: "Графика",
    installed: true,
    icon: "🌈",
  },
  {
    id: "5",
    name: "JourneyMap",
    author: "techbrew",
    description: "Полноэкранная и миникарта с отметками и маршрутами",
    downloads: "1.5M",
    rating: 4.6,
    category: "Утилиты",
    installed: false,
    icon: "🗺️",
  },
  {
    id: "6",
    name: "Create",
    author: "simibubi",
    description: "Механизмы, конвейеры, поезда и автоматизация в стиле стимпанк",
    downloads: "2.3M",
    rating: 4.9,
    category: "Контент",
    installed: false,
    icon: "⚙️",
  },
];

const categories = ["Все", "Оптимизация", "Графика", "Утилиты", "Контент", "PvP"];

export function ModsPage() {
  return (
    <div className="animate-fade-in space-y-6">
      <div>
        <h1 className="text-2xl font-bold text-wenz-text">Моды</h1>
        <p className="mt-1 text-sm text-wenz-muted">
          Устанавливай и управляй модами
        </p>
      </div>

      {/* Search & Filters */}
      <div className="flex items-center gap-3">
        <div className="relative flex-1">
          <Search className="absolute left-3 top-1/2 h-4 w-4 -translate-y-1/2 text-wenz-muted" />
          <input
            type="text"
            placeholder="Поиск модов..."
            className="w-full rounded-lg border border-wenz-border bg-wenz-card py-2.5 pl-10 pr-4 text-sm text-wenz-text placeholder-wenz-muted outline-none transition-colors focus:border-wenz-accent/50"
          />
        </div>
        <button className="flex items-center gap-2 rounded-lg border border-wenz-border bg-wenz-card px-4 py-2.5 text-sm text-wenz-muted transition-colors hover:border-wenz-accent/30 hover:text-wenz-text">
          <Filter className="h-4 w-4" />
          Фильтры
        </button>
      </div>

      {/* Categories */}
      <div className="flex gap-2">
        {categories.map((cat, i) => (
          <button
            key={cat}
            className={`rounded-lg px-3 py-1.5 text-xs font-medium transition-colors ${
              i === 0
                ? "bg-wenz-accent/20 text-wenz-accent"
                : "bg-wenz-card text-wenz-muted hover:bg-white/5 hover:text-wenz-text"
            }`}
          >
            {cat}
          </button>
        ))}
      </div>

      {/* Mods Grid */}
      <div className="grid grid-cols-2 gap-4">
        {mods.map((mod) => (
          <div
            key={mod.id}
            className="rounded-xl border border-wenz-border bg-wenz-card p-4 transition-all hover:border-wenz-accent/20"
          >
            <div className="flex gap-4">
              <div className="flex h-12 w-12 flex-shrink-0 items-center justify-center rounded-lg bg-wenz-bg text-2xl">
                {mod.icon}
              </div>
              <div className="flex-1 overflow-hidden">
                <div className="flex items-start justify-between">
                  <div>
                    <h3 className="font-semibold text-wenz-text">{mod.name}</h3>
                    <p className="text-xs text-wenz-muted">от {mod.author}</p>
                  </div>
                  <span className="rounded bg-wenz-bg px-2 py-0.5 text-xs text-wenz-muted">
                    {mod.category}
                  </span>
                </div>
                <p className="mt-2 text-xs leading-relaxed text-wenz-muted">
                  {mod.description}
                </p>
                <div className="mt-3 flex items-center justify-between">
                  <div className="flex items-center gap-3">
                    <span className="flex items-center gap-1 text-xs text-wenz-muted">
                      <Download className="h-3 w-3" />
                      {mod.downloads}
                    </span>
                    <span className="flex items-center gap-1 text-xs text-wenz-warning">
                      <Star className="h-3 w-3" fill="currentColor" />
                      {mod.rating}
                    </span>
                  </div>
                  {mod.installed ? (
                    <span className="flex items-center gap-1 rounded-lg bg-wenz-success/10 px-3 py-1 text-xs font-medium text-wenz-success">
                      <Check className="h-3 w-3" />
                      Установлен
                    </span>
                  ) : (
                    <button className="rounded-lg bg-wenz-accent/10 px-3 py-1 text-xs font-medium text-wenz-accent transition-colors hover:bg-wenz-accent/20">
                      Установить
                    </button>
                  )}
                </div>
              </div>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}
