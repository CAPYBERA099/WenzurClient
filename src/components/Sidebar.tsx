import {
  Home,
  Package,
  Puzzle,
  Shirt,
  Settings,
  LogOut,
  Gamepad2,
} from "lucide-react";
import type { Account } from "../App";

interface Props {
  currentPage: string;
  onPageChange: (page: string) => void;
  account: Account;
  onLogout: () => void;
}

const NAV_ITEMS = [
  { id: "home", icon: Home, label: "Главная" },
  { id: "instances", icon: Package, label: "Версии" },
  { id: "mods", icon: Puzzle, label: "Моды" },
  { id: "skins", icon: Shirt, label: "Скины" },
  { id: "settings", icon: Settings, label: "Настройки" },
];

export default function Sidebar({ currentPage, onPageChange, account, onLogout }: Props) {
  const skinHeadUrl = `https://skinsystem.ely.by/skins/${account.username}.png`;

  return (
    <div className="w-[220px] bg-wenz-surface border-r border-wenz-border flex flex-col">
      {/* Logo */}
      <div className="px-4 py-5 border-b border-wenz-border">
        <div className="flex items-center gap-2.5">
          <div className="w-9 h-9 rounded-lg bg-gradient-to-br from-wenz-accent/30 to-wenz-accent/10 flex items-center justify-center border border-wenz-accent/20">
            <Gamepad2 size={18} className="text-wenz-accent" />
          </div>
          <div>
            <p className="text-white font-bold text-sm">WenzLauncher</p>
            <p className="text-gray-500 text-[10px]">v0.1.0</p>
          </div>
        </div>
      </div>

      {/* Navigation */}
      <nav className="flex-1 px-3 py-4 space-y-1">
        {NAV_ITEMS.map(({ id, icon: Icon, label }) => (
          <button
            key={id}
            onClick={() => onPageChange(id)}
            className={`w-full flex items-center gap-3 px-3 py-2.5 rounded-lg text-sm font-medium transition-all duration-200 ${
              currentPage === id
                ? "bg-wenz-accent/15 text-wenz-accent"
                : "text-gray-400 hover:text-white hover:bg-white/5"
            }`}
          >
            <Icon size={18} />
            <span>{label}</span>
          </button>
        ))}
      </nav>

      {/* Account */}
      <div className="px-3 pb-4">
        <div className="bg-wenz-bg/80 rounded-xl p-3 border border-wenz-border">
          <div className="flex items-center gap-3 mb-3">
            {/* Player head from skin */}
            <div className="w-9 h-9 rounded-lg overflow-hidden bg-wenz-surface border border-wenz-border flex-shrink-0">
              <img
                src={skinHeadUrl}
                alt={account.username}
                className="w-full h-full object-cover"
                style={{ imageRendering: "pixelated", objectPosition: "top" }}
                onError={(e) => {
                  (e.target as HTMLImageElement).style.display = 'none';
                }}
              />
            </div>
            <div className="min-w-0">
              <p className="text-white text-sm font-medium truncate">{account.username}</p>
              <p className="text-gray-500 text-[10px]">ely.by</p>
            </div>
          </div>
          <button
            onClick={onLogout}
            className="w-full flex items-center justify-center gap-2 text-xs text-gray-500 hover:text-red-400 py-1.5 rounded-lg hover:bg-red-500/10 transition-colors"
          >
            <LogOut size={12} />
            Выйти
          </button>
        </div>
      </div>
    </div>
  );
}
