import { useState } from "react";
import { Search, Download, ExternalLink, Package, Star, Filter } from "lucide-react";

interface Mod {
  id: string;
  name: string;
  description: string;
  author: string;
  downloads: string;
  category: string;
  icon: string;
  url: string;
}

const POPULAR_MODS: Mod[] = [
  {
    id: "optifine",
    name: "OptiFine",
    description: "Оптимизация FPS, шейдеры, HD текстуры, динамическое освещение",
    author: "sp614x",
    downloads: "200M+",
    category: "Оптимизация",
    icon: "⚡",
    url: "https://optifine.net",
  },
  {
    id: "sodium",
    name: "Sodium",
    description: "Мощная оптимизация рендеринга для Fabric/NeoForge",
    author: "CaffeineMC",
    downloads: "50M+",
    category: "Оптимизация",
    icon: "🧪",
    url: "https://modrinth.com/mod/sodium",
  },
  {
    id: "fabric-api",
    name: "Fabric API",
    description: "Базовая библиотека для модов на Fabric",
    author: "FabricMC",
    downloads: "150M+",
    category: "Библиотека",
    icon: "🧵",
    url: "https://modrinth.com/mod/fabric-api",
  },
  {
    id: "iris",
    name: "Iris Shaders",
    description: "Шейдеры для Fabric/NeoForge, совместимы с Sodium",
    author: "IrisShaders",
    downloads: "30M+",
    category: "Шейдеры",
    icon: "🌈",
    url: "https://modrinth.com/mod/iris",
  },
  {
    id: "litematica",
    name: "Litematica",
    description: "Схемы построек, голограммы, визуализация",
    author: "masa",
    downloads: "15M+",
    category: "Утилиты",
    icon: "📐",
    url: "https://modrinth.com/mod/litematica",
  },
  {
    id: "replay-mod",
    name: "Replay Mod",
    description: "Запись и воспроизведение геймплея, кинематографические камеры",
    author: "CrushedPixel",
    downloads: "10M+",
    category: "Утилиты",
    icon: "🎬",
    url: "https://modrinth.com/mod/replaymod",
  },
  {
    id: "jei",
    name: "JEI (Just Enough Items)",
    description: "Просмотр рецептов крафта, справочник предметов",
    author: "mezz",
    downloads: "100M+",
    category: "Утилиты",
    icon: "📖",
    url: "https://modrinth.com/mod/jei",
  },
  {
    id: "xaeros-minimap",
    name: "Xaero's Minimap",
    description: "Мини-карта с вейпоинтами, пещерный режим, карта мира",
    author: "xaero96",
    downloads: "40M+",
    category: "Карта",
    icon: "🗺️",
    url: "https://modrinth.com/mod/xaeros-minimap",
  },
];

const CATEGORIES = ["Все", "Оптимизация", "Шейдеры", "Утилиты", "Библиотека", "Карта"];

export default function ModsPage() {
  const [search, setSearch] = useState("");
  const [category, setCategory] = useState("Все");

  const filtered = POPULAR_MODS.filter((mod) => {
    if (search && !mod.name.toLowerCase().includes(search.toLowerCase())) return false;
    if (category !== "Все" && mod.category !== category) return false;
    return true;
  });

  return (
    <div className="p-6 space-y-6">
      <div>
        <h1 className="text-2xl font-bold text-white">Моды</h1>
        <p className="text-gray-400 text-sm mt-1">
          Популярные моды для Minecraft. Скачивайте с Modrinth.
        </p>
      </div>

      {/* Search + categories */}
      <div className="flex gap-3 flex-wrap">
        <div className="flex-1 min-w-[200px] relative">
          <Search size={16} className="absolute left-3 top-1/2 -translate-y-1/2 text-gray-500" />
          <input
            type="text"
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            placeholder="Найти мод..."
            className="w-full bg-wenz-surface border border-wenz-border rounded-lg pl-10 pr-4 py-2.5 text-white text-sm placeholder-gray-500 focus:outline-none focus:border-wenz-accent/50"
          />
        </div>
        <div className="flex gap-1 bg-wenz-surface border border-wenz-border rounded-lg p-1">
          {CATEGORIES.map((cat) => (
            <button
              key={cat}
              onClick={() => setCategory(cat)}
              className={`px-3 py-1.5 text-xs rounded-md font-medium transition-colors ${
                category === cat
                  ? "bg-wenz-accent/20 text-wenz-accent"
                  : "text-gray-400 hover:text-white"
              }`}
            >
              {cat}
            </button>
          ))}
        </div>
      </div>

      {/* Mod cards */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        {filtered.map((mod) => (
          <div
            key={mod.id}
            className="bg-wenz-surface border border-wenz-border rounded-2xl p-5 hover:border-wenz-accent/20 transition-colors group"
          >
            <div className="flex items-start gap-4">
              <div className="text-3xl">{mod.icon}</div>
              <div className="flex-1 min-w-0">
                <div className="flex items-center gap-2 mb-1">
                  <h3 className="text-white font-semibold">{mod.name}</h3>
                  <span className="text-xs bg-wenz-accent/10 text-wenz-accent px-2 py-0.5 rounded-full">
                    {mod.category}
                  </span>
                </div>
                <p className="text-gray-400 text-sm mb-2">{mod.description}</p>
                <div className="flex items-center gap-4 text-xs text-gray-500">
                  <span>by {mod.author}</span>
                  <span className="flex items-center gap-1">
                    <Download size={11} />
                    {mod.downloads}
                  </span>
                </div>
              </div>
            </div>
            <div className="flex justify-end mt-3">
              <a
                href={mod.url}
                target="_blank"
                rel="noopener noreferrer"
                className="flex items-center gap-1.5 bg-wenz-accent/10 hover:bg-wenz-accent/20 text-wenz-accent rounded-lg px-3 py-1.5 text-sm transition-colors"
              >
                <ExternalLink size={14} />
                Открыть
              </a>
            </div>
          </div>
        ))}
      </div>

      {filtered.length === 0 && (
        <div className="text-center py-12 text-gray-500">
          <Package size={40} className="mx-auto mb-3 opacity-50" />
          <p>Моды не найдены</p>
        </div>
      )}

      {/* Modrinth link */}
      <div className="bg-wenz-surface/50 border border-wenz-border rounded-2xl p-6 text-center">
        <p className="text-gray-400 text-sm mb-3">
          Больше модов на Modrinth — крупнейшей платформе модов для Minecraft
        </p>
        <a
          href="https://modrinth.com/mods"
          target="_blank"
          rel="noopener noreferrer"
          className="inline-flex items-center gap-2 bg-green-500/20 hover:bg-green-500/30 text-green-400 rounded-xl px-5 py-2.5 text-sm font-medium transition-colors"
        >
          <ExternalLink size={16} />
          Открыть Modrinth
        </a>
      </div>
    </div>
  );
}
