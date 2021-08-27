"use strict";(self.webpackChunkmy_website=self.webpackChunkmy_website||[]).push([[6267],{3905:function(e,t,n){n.d(t,{Zo:function(){return h},kt:function(){return m}});var a=n(7294);function i(e,t,n){return t in e?Object.defineProperty(e,t,{value:n,enumerable:!0,configurable:!0,writable:!0}):e[t]=n,e}function o(e,t){var n=Object.keys(e);if(Object.getOwnPropertySymbols){var a=Object.getOwnPropertySymbols(e);t&&(a=a.filter((function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable}))),n.push.apply(n,a)}return n}function r(e){for(var t=1;t<arguments.length;t++){var n=null!=arguments[t]?arguments[t]:{};t%2?o(Object(n),!0).forEach((function(t){i(e,t,n[t])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(n)):o(Object(n)).forEach((function(t){Object.defineProperty(e,t,Object.getOwnPropertyDescriptor(n,t))}))}return e}function c(e,t){if(null==e)return{};var n,a,i=function(e,t){if(null==e)return{};var n,a,i={},o=Object.keys(e);for(a=0;a<o.length;a++)n=o[a],t.indexOf(n)>=0||(i[n]=e[n]);return i}(e,t);if(Object.getOwnPropertySymbols){var o=Object.getOwnPropertySymbols(e);for(a=0;a<o.length;a++)n=o[a],t.indexOf(n)>=0||Object.prototype.propertyIsEnumerable.call(e,n)&&(i[n]=e[n])}return i}var l=a.createContext({}),s=function(e){var t=a.useContext(l),n=t;return e&&(n="function"==typeof e?e(t):r(r({},t),e)),n},h=function(e){var t=s(e.components);return a.createElement(l.Provider,{value:t},e.children)},d={inlineCode:"code",wrapper:function(e){var t=e.children;return a.createElement(a.Fragment,{},t)}},u=a.forwardRef((function(e,t){var n=e.components,i=e.mdxType,o=e.originalType,l=e.parentName,h=c(e,["components","mdxType","originalType","parentName"]),u=s(n),m=i,p=u["".concat(l,".").concat(m)]||u[m]||d[m]||o;return n?a.createElement(p,r(r({ref:t},h),{},{components:n})):a.createElement(p,r({ref:t},h))}));function m(e,t){var n=arguments,i=t&&t.mdxType;if("string"==typeof e||i){var o=n.length,r=new Array(o);r[0]=u;var c={};for(var l in t)hasOwnProperty.call(t,l)&&(c[l]=t[l]);c.originalType=e,c.mdxType="string"==typeof e?e:i,r[1]=c;for(var s=2;s<o;s++)r[s]=n[s];return a.createElement.apply(null,r)}return a.createElement.apply(null,n)}u.displayName="MDXCreateElement"},4753:function(e,t,n){n.r(t),n.d(t,{frontMatter:function(){return c},contentTitle:function(){return l},metadata:function(){return s},toc:function(){return h},default:function(){return u}});var a=n(7462),i=n(3366),o=(n(7294),n(3905)),r=["components"],c={id:"HybridCache",title:"HybridCache"},l=void 0,s={unversionedId:"Cache_Library_User_Guides/HybridCache",id:"Cache_Library_User_Guides/HybridCache",isDocsHomePage:!1,title:"HybridCache",description:"HybridCache feature enables CacheAllocator to extend the DRAM cache to NVM. With HybridCache, cachelib can seamlessly move Items stored in cache across DRAM and NVM as they are accessed. Using HybridCache, you can shrink your DRAM footprint of the cache and replace it with NVM like Flash. This can also enable you to achieve large cache capacities for the same or relatively lower power and dollar cost.",source:"@site/docs/Cache_Library_User_Guides/HybridCache.md",sourceDirName:"Cache_Library_User_Guides",slug:"/Cache_Library_User_Guides/HybridCache",permalink:"/docs/Cache_Library_User_Guides/HybridCache",editUrl:"https://github.com/facebook/docusaurus/edit/master/website/docs/Cache_Library_User_Guides/HybridCache.md",tags:[],version:"current",frontMatter:{id:"HybridCache",title:"HybridCache"},sidebar:"someSidebar",previous:{title:"automatic pool resizing",permalink:"/docs/Cache_Library_User_Guides/automatic_pool_resizing"},next:{title:"Configure HybridCache",permalink:"/docs/Cache_Library_User_Guides/Configure_HybridCache"}},h=[{value:"Configuration",id:"configuration",children:[]},{value:"Allocating items",id:"allocating-items",children:[]},{value:"Accessing items on cache",id:"accessing-items-on-cache",children:[]}],d={toc:h};function u(e){var t=e.components,n=(0,i.Z)(e,r);return(0,o.kt)("wrapper",(0,a.Z)({},d,n,{components:t,mdxType:"MDXLayout"}),(0,o.kt)("p",null,"HybridCache feature enables ",(0,o.kt)("inlineCode",{parentName:"p"},"CacheAllocator")," to extend the DRAM cache to NVM. With HybridCache, cachelib can seamlessly move Items stored in cache across DRAM and NVM as they are accessed. Using HybridCache, you can shrink your DRAM footprint of the cache and replace it with NVM like Flash. This can also enable you to achieve large cache capacities for the same or relatively lower power and dollar cost."),(0,o.kt)("h1",{id:"design"},"Design"),(0,o.kt)("p",null,"When you use HybridCache, items allocated in the cache can live on NVM or DRAM based on how they are accessed. Irrespective of where they are, ",(0,o.kt)("strong",{parentName:"p"},"when you access them, you always get them to be in DRAM"),"."),(0,o.kt)("p",null,"Items start their lifetime on DRAM when you ",(0,o.kt)("inlineCode",{parentName:"p"},"allocate()"),". As an item becomes cold it gets evicted from DRAM when the cache is full. ",(0,o.kt)("inlineCode",{parentName:"p"},"CacheAllocator")," spills it to a cache on the NVM device. Upon subsequent access through ",(0,o.kt)("inlineCode",{parentName:"p"},"find()"),", if the item is not in DRAM, cachelib looks it up in the HybridCache and if found, moves it to DRAM. When the HybridCache gets filled up, subsequent insertions into the HybridCache from DRAM  will throw away colder items from HybridCache. ",(0,o.kt)("inlineCode",{parentName:"p"},"CacheAllocator")," manages the synchronization of the lifetime of an item across RAM and NVM for all cachelib APIs like",(0,o.kt)("inlineCode",{parentName:"p"},"insertOrReplace()"),", ",(0,o.kt)("inlineCode",{parentName:"p"},"remove()"),", ",(0,o.kt)("inlineCode",{parentName:"p"},"allocate()"),", and ",(0,o.kt)("inlineCode",{parentName:"p"},"find()"),". Conceptually you can imagine your cache to span across DRAM and NVM as one big cache similar to how an OS would use virtual memory to extend your main memory using additional SWAP space."),(0,o.kt)("h1",{id:"using-hybridcache"},"Using HybridCache"),(0,o.kt)("p",null,"The following sections describe how to configure HybridCache, how to allocate memory from the cache to store items, and how to access the items on the cache."),(0,o.kt)("h2",{id:"configuration"},"Configuration"),(0,o.kt)("p",null,"To use HybridCache, you need to set the appropriate options in the cache's config under the ",(0,o.kt)("inlineCode",{parentName:"p"},"nvmConfig")," section. Refer to ",(0,o.kt)("a",{parentName:"p",href:"Configure_HybridCache/"},"Configure HybridCache")," for a list of options to configure for the HybridCache's backing engine."),(0,o.kt)("p",null,"For example:"),(0,o.kt)("pre",null,(0,o.kt)("code",{parentName:"pre",className:"language-cpp"},"nvmConfig.navyConfig.set<OPTION NAME> = <YOUR OPTION VALUE>;\n\n// set the nvmConfig in the main cache config.\nlruAllocatorConfig.enableHybridCache(nvmConfig);\n")),(0,o.kt)("p",null,"After seting up a HybridCache, you can use the cache almost the same way as a pure DRAM based cache."),(0,o.kt)("h2",{id:"allocating-items"},"Allocating items"),(0,o.kt)("p",null,"Use the ",(0,o.kt)("inlineCode",{parentName:"p"},"allocate()")," API to allocate memory for an item and get an ",(0,o.kt)("inlineCode",{parentName:"p"},"ItemHandle"),":"),(0,o.kt)("pre",null,(0,o.kt)("code",{parentName:"pre",className:"language-cpp"},'// Allocate memory for the item.\nauto handle = cache.allocate(pool_id, "foobar", my_data.size());\n\n// Initialize the item\nstd::memcpy(handle->getMemory(), my_data, my_data.size());\n\n// Make the item accessible by inserting it into the cache.\ncache.insertOrReplace(handle);\n')),(0,o.kt)("h2",{id:"accessing-items-on-cache"},"Accessing items on cache"),(0,o.kt)("p",null,"When you call the ",(0,o.kt)("inlineCode",{parentName:"p"},"find()")," API to look up an item by it key, cachelib returns a handle as before. However, the handle might not be immediately ",(0,o.kt)("em",{parentName:"p"},"ready")," to dereference and access the item's memory.  CacheLib will promote the item from NVM to DRAM and notify the handle to be ready. Note that",(0,o.kt)("inlineCode",{parentName:"p"},"Handle")," will always eventually become ready. Cachelib provides the following ",(0,o.kt)("inlineCode",{parentName:"p"},"Handle"),"  states and corresponding APIs to distinguish the semantics"),(0,o.kt)("ol",null,(0,o.kt)("li",{parentName:"ol"},(0,o.kt)("strong",{parentName:"li"},"Ready"),"\nThis indicates that the item is in DRAM and accessing the Item's memory through the ",(0,o.kt)("inlineCode",{parentName:"li"},"Handle")," will not block."),(0,o.kt)("li",{parentName:"ol"},(0,o.kt)("strong",{parentName:"li"},"Not Ready"),"\nThis indicates that the handle is still waiting for the item to be pulled into DRAM. Accessing the item through ",(0,o.kt)("inlineCode",{parentName:"li"},"Handle")," will block.")),(0,o.kt)("p",null,"To check whether a Handle is ready, call the ",(0,o.kt)("inlineCode",{parentName:"p"},"isReady()")," method via the Handle."),(0,o.kt)("p",null,"If the application can tolerate the latency of accessing NVM for some of your accesses from cache, there is not a lot of change that is needed on how you use cachelib today. However, if you are impacted by latency, you can do the following:"),(0,o.kt)("ol",null,(0,o.kt)("li",{parentName:"ol"},"By default, if you don't change your existing cachelib code, dereferencing a handle that is not ready or doing any check on the result of the handle will ",(0,o.kt)("strong",{parentName:"li"},"block")," until it becomes ready. When it is blocking, if you are in a fiber context, it puts the fiber to sleep."),(0,o.kt)("li",{parentName:"ol"},"Set a callback to be executed ",(0,o.kt)("inlineCode",{parentName:"li"},"onReady()"),". The callback will be executed when the handle is ready or immediately if the handle is already ready."),(0,o.kt)("li",{parentName:"ol"},"Get a ",(0,o.kt)("inlineCode",{parentName:"li"},"folly::SemiFuture")," on the handle by calling the ",(0,o.kt)("inlineCode",{parentName:"li"},"handleToSemiFuture()")," method."),(0,o.kt)("li",{parentName:"ol"},"Pass the handle in its current state to another execution context that can then execute your code when the handle becomes ready.")),(0,o.kt)("p",null,"The following code snipper highlights the various techniques."),(0,o.kt)("pre",null,(0,o.kt)("code",{parentName:"pre",className:"language-cpp"},'auto processItem  = [](Handle h) {\n  // my processing logic.\n};\n\n/* Accessing item on a fiber or in blocking way with NvmCache */\nauto handle = cache.find("foobar");\n\n/* This will block or switch your fiber if item is in NVM, until it is ready. */\nif (handle) {\n  std::cout << handle->getMemoryAs<const char*>(), handle->getSize();\n}\n\n/* Accessing item and getting a future */\nauto handle = cache.find("foobar");\nauto semiFuture = util::handleToSemiFuture(std::move(handle));\n\n/* Accessing an item and setting a onReady callback */\nauto handle = cache.find("foobar");\nhandle.onReady(processItem); // process the item when the handle becomes ready\n')),(0,o.kt)("h1",{id:"features-that-differ-in-semantics"},"Features that differ in semantics"),(0,o.kt)("p",null,(0,o.kt)("inlineCode",{parentName:"p"},"HybridCache")," is in active development. Not all DRAM cache features are correspondingly available in ",(0,o.kt)("inlineCode",{parentName:"p"},"HybridCache")," setup. Some notable ones to consider :"),(0,o.kt)("ul",null,(0,o.kt)("li",{parentName:"ul"},"Pools only partition within DRAM, but not on HybridCache.  Items are sticky to their pools when they are moved to/from NVM."),(0,o.kt)("li",{parentName:"ul"},"The ",(0,o.kt)("inlineCode",{parentName:"li"},"insert()")," API cannot be used with HybridCache."),(0,o.kt)("li",{parentName:"ul"},"Iteration of the cache only iterates over the DRAM part."),(0,o.kt)("li",{parentName:"ul"},"When looking at the stats, some stats are broken down by DRAM and NVM. The semantics of getting a uniform stats across both mediums is currently not supported.")))}u.isMDXComponent=!0}}]);