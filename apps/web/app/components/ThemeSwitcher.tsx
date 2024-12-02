import { useEffect, useState } from "react";
import { FaMoon, FaSun } from "react-icons/fa";
import { Button } from "./Button";


export const ThemeSwitcher = () => {

    const [isDark, setIsDark] = useState(true);

    useEffect(() => {
        const isDark = document.documentElement.classList.contains("dark")
        setIsDark(isDark)
    }, [])

    const changeTheme = () => {
        console.log('changing theme')
        document.documentElement.classList.toggle("dark");
        setIsDark((dark) => {
            const newTheme = !dark;
            localStorage.setItem('@app/theme', newTheme ? 'dark' : 'light')
            return !newTheme
        })
    };

    return (<Button
        type="button"
        className="flex flex-row gap-2 dark:!text-colors-hf-beige !text-colors-hf-brown"
        variant="link"
        onClick={() => { changeTheme() }}>
        {isDark ? <FaMoon /> : <FaSun />} Change theme
    </Button>);
}
