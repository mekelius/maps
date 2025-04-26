import { defineConfig } from 'vitepress'
import { withSidebar } from 'vitepress-sidebar';

// https://vitepress.dev/reference/site-config
export default defineConfig(withSidebar({
  title: "Maps programming language",
  description: "Maps design docs",

  themeConfig: {
    // https://vitepress.dev/reference/default-theme-config
    socialLinks: [
      { icon: 'github', link: 'https://github.com/mekelius/maps' }
    ]
  },

  plugins: [
  ],

}, {
  documentRootPath: '/',
  collapsed: false,
  capitalizeFirst: true
}))
