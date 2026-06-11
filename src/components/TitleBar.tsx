import { Minus, Square, X } from "lucide-react";

export function TitleBar() {
  return (
    <div
      data-tauri-drag-region
      className="flex h-10 items-center justify-between border-b border-wenz-border bg-wenz-card px-4"
    >
      <div className="flex items-center gap-2" data-tauri-drag-region>
        <div className="flex h-6 w-6 items-center justify-center rounded bg-wenz-accent/20">
          <span className="text-xs font-bold text-wenz-accent">W</span>
        </div>
        <span className="text-sm font-semibold text-wenz-text" data-tauri-drag-region>
          WenzLauncher
        </span>
        <span className="text-xs text-wenz-muted" data-tauri-drag-region>
          v0.1.0
        </span>
      </div>

      <div className="flex items-center gap-1">
        <button className="flex h-7 w-7 items-center justify-center rounded transition-colors hover:bg-white/10">
          <Minus className="h-3.5 w-3.5 text-wenz-muted" />
        </button>
        <button className="flex h-7 w-7 items-center justify-center rounded transition-colors hover:bg-white/10">
          <Square className="h-3 w-3 text-wenz-muted" />
        </button>
        <button className="flex h-7 w-7 items-center justify-center rounded transition-colors hover:bg-red-500/80">
          <X className="h-3.5 w-3.5 text-wenz-muted" />
        </button>
      </div>
    </div>
  );
}
