import { createCookieSessionStorage } from "@remix-run/node";

type SessionData = {
    token: string;
    username: string;
};

type SessionFlashData = {
    message: {
        type: "error" | "success" | "warn";
        message: string;
    }
};


export const sessionStorage = createCookieSessionStorage<SessionData, SessionFlashData>({
    cookie: {
        name: "__session",
        secure: false,
        secrets: [process.env.SESSION_SECRET!],
        sameSite: "lax",
        path: "/",
        httpOnly: true,
    },
});

export const { getSession, commitSession, destroySession } = sessionStorage;