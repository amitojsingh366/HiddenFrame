const savedTheme = localStorage.getItem('@app/theme')
if (savedTheme === 'dark' || ((!savedTheme) && window.matchMedia('(prefers-color-scheme: dark)').matches)) {
    document.documentElement.classList.add('dark')
} else {
    document.documentElement.classList.remove('dark')
}