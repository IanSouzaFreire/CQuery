function importModule(url) {
  return new Promise((resolve, reject) => {
    const script = document.createElement('script');
    script.src = url;
    script.onload = resolve;
    script.onerror = reject;
    document.head.appendChild(script);
  });
}

function importCSS(url) {
  return new Promise((resolve, reject) => {
    const link = document.createElement('link');
    link.rel = 'stylesheet';
    link.href = url;
    link.onload = resolve;
    link.onerror = reject;
    document.head.appendChild(link);
  });
}

async function importModules() {
  const modules = [
    'https://code.jquery.com/jquery-3.7.1.js',
    'https://cdn.jsdelivr.net/npm/denali-css@2.5.1/index.min.js',
  ];
  
  const importPromises = modules.map(url => importModule(url));
  await Promise.all(importPromises);
}

async function importStyles() {
  const styles = [
    'https://cdn.jsdelivr.net/npm/denali-css@2.5.1/css/denali.min.css',
  ];

  const importPromises = styles.map(url => importCSS(url));
  await Promise.all(importPromises);
}

async function initializeLibraries() {
  console.log('All libraries initialized');
}

Promise.all([importModules(), importStyles()])
  .then(() => {
    console.log('All modules and styles imported');
    return initializeLibraries();
  })
  .then(() => {
    console.log('Libraries initialized');
    document.dispatchEvent(new Event('importsComplete'));
  })
  .catch(error => {
    console.error('Error during import or initialization:', error);
  });
