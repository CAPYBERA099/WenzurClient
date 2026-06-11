import { NavLink } from "react-router-dom";
import {
  Home,
  Layers,
  Puzzle,
  Shirt,
  Settings,
  User,
  ChevronRight,
} from "lucide-react";
import { cn } from "../lib/utils";

const navItems = [
  { to: "/", icon: Home, label: "Главная" },
  { to: "/instances", icon: Layers, label: "Сборки" },
  { to: "/mods", icon: Puzzle, label: "Моды" },
  { to: "/skins", icon: Shirt, label: "Скины" },
  { to: "/settings", icon: Settings, label: "Настройки" },
];

export function Sidebar() {
  return (
    <aside className="flex w-56 flex-col border-r border-wenz-border bg-wenz-card">
      {/* Player Card */}
      <div className="border-b border-wenz-border p-4">
        <div className="flex items-center gap-3">
          <div className="flex h-10 w-10 items-center justify-center rounded-lg bg-wenz-accent/20">
            <User className="h-5 w-5 text-wenz-accent" />
          </div>
          <div className="flex-1 overflow-hidden">
            <p className="truncate text-sm font-semibold text-wenz-text">
              WenzPlayer
            </p>
            <p className="text-xs text-wenz-success">● В сети</p>
          </div>
        </div>
      </div>

      {/* Navigation */}
      <nav className="flex-1 space-y-1 p-3">
        {navItems.map((item) => (
          <NavLink
            key={item.to}
            to={item.to}
            end={item.to === "/"}
            className={({ isActive }) =>
              cn(
                "flex items-center gap-3 rounded-lg px-3 py-2.5 text-sm font-medium transition-all",
                isActive
                  ? "bg-wenz-accent/10 text-wenz-accent"
                  : "text-wenz-muted hover:bg-white/5 hover:text-wenz-text"
              )
            }
          >
            {({ isActive }) => (
              <>
                <item.icon className="h-4 w-4 flex-shrink-0" />
                <span className="flex-1">{item.label}</span>
                {isActive && (
                  <ChevronRight className="h-3.5 w-3.5 text-wenz-accent/60" />
                )}
              </>
            )}
          </NavLink>
        ))}
      </nav>

      {/* Bottom Status */}
      <div className="border-t border-wenz-border p-3">
        <div className="rounded-lg bg-wenz-bg/50 px-3 py-2">
          <div className="flex items-center justify-between text-xs">
            <span className="text-wenz-muted">RAM</span>
            <span className="font-mono text-wenz-accent">4096 MB</span>
          </div>
          <div className="mt-1.5 h-1 rounded-full bg-wenz-border">
            <div className="h-1 w-1/2 rounded-full bg-wenz-accent" />
          </div>
        </div>
      </div>
    </aside>
  );
}
