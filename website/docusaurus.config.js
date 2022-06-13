/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

module.exports = {
  title: 'CacheLib',
  tagline: 'Pluggable caching engine to build and scale high performance cache services',
  favicon: 'img/favicon.ico',

  // Settings for the production GitHub repository WITH custom domain
  // The website will be https://cachelib.org/
  // Don't forget a corrsponding CNAME file in the 'static' directory.
  url: 'https://cachelib.org',
  baseUrl: '/',
  organizationName: 'facebook', // Usually your GitHub org/user name.
  projectName: 'CacheLib', // Usually your repo name.


  themeConfig: {
    image: 'img/CacheLib-Logo-small.png',
    navbar: {
      title: 'CacheLib',
      logo: {
        alt: 'My Facebook Project Logo',
        src: 'img/CacheLib-Logo-small.png',
      },
	items: [
        {
          to: 'docs/installation/installation',
          activeBasePath: 'docs',
          label: 'Build and Installation',
          position: 'left',
        },
        {
          to: 'docs/',
          activeBasePath: 'docs',
          label: 'API and Usage',
          position: 'left',
        },
        {
          to: 'docs/Cache_Library_User_Guides/Cachebench_Overview',
          activeBasePath: 'docs',
          label: 'Cachebench',
          position: 'left',
        },
        {
          to: 'learnmore', // temporarily redirect here until we add docs.
          activeBasePath: 'learnmore',
          label: 'Architecture Guide',
          position: 'left',
        },
        {
          to: 'learnmore/',
          activeBasePath: 'learnmore',
          label: 'Learn More',
          position: 'right',
        },
        // Please keep GitHub link to the right for consistency.
        {
          href: 'https://github.com/facebook/CacheLib',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Reach Us',
          items: [
            {
              label: 'GitHub',
              href: 'https://github.com/facebook/CacheLib',
            },
          ],
        },
        {
          title: 'Community',
          items: [
            {
              label: 'Facebook Developer Page',
		href: 'https://www.facebook.com/cachelib/',
            },
            {
              label: 'Twitter',
              href: 'https://twitter.com/MetaOpenSource',
            },
          ],
        },
	  /*
        {
          title: 'More',
          items: [
            {
              label: 'Blog',
              to: 'blog',
            },
            {
              label: 'GitHub',
              href: 'https://github.com/facebook/docusaurus',
            },
          ],
        },
*/
        {
          title: 'Legal',
          // Please do not remove the privacy and terms, it's a legal requirement.
          items: [
            {
              label: 'Privacy',
              href: 'https://opensource.facebook.com/legal/privacy/',
            },
            {
              label: 'Terms',
              href: 'https://opensource.facebook.com/legal/terms/',
            },
          ],
        },
      ],
      logo: {
        alt: 'Facebook Open Source Logo',
        src: 'img/oss_logo.png',
        href: 'https://opensource.facebook.com',
      },
      // Please do not remove the credits, help to publicize Docusaurus :)
      copyright: `Copyright © ${new Date().getFullYear()} Facebook, Inc. Built with Docusaurus.`,
    },
  },
  presets: [
    [
      '@docusaurus/preset-classic',
      {
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
          // Please change this to your repo.
          editUrl:
            'https://github.com/facebook/docusaurus/edit/master/website/',
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      },
    ],
  ],
};
