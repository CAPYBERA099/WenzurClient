declare module "skinview3d" {
  export class SkinViewer {
    constructor(options: {
      canvas: HTMLCanvasElement;
      width: number;
      height: number;
      skin?: string;
      cape?: string;
    });
    camera: { rotation: { x: number; y: number; z: number }; position: { x: number; y: number; z: number } };
    zoom: number;
    animation: any;
    autoRotate: boolean;
    autoRotateSpeed: number;
    background: any;
    controls: {
      enableRotate: boolean;
      enableZoom: boolean;
      enablePan: boolean;
    };
    loadSkin(url: string, options?: any): Promise<void>;
    loadCape(url: string, options?: any): Promise<void>;
    dispose(): void;
  }

  export class WalkingAnimation {
    speed: number;
  }

  export class IdleAnimation {
    speed: number;
  }

  export class RunningAnimation {
    speed: number;
  }
}
