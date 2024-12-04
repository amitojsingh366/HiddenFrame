/* eslint-disable @typescript-eslint/no-explicit-any */
import type { LoaderFunction, MetaFunction, ActionFunction } from "@remix-run/node";
import { redirect } from "@remix-run/node";
import { PageHeader, Button, Input } from "../../components";
import { getSession } from "../../session";
import { Form, useActionData, useLoaderData, useNavigation } from "@remix-run/react";
import { useEffect, useMemo, useState } from "react";
import { BASE_API_URL } from "../../lib/consts";
import { toast } from "sonner";
import { Copy } from "lucide-react";

export const meta: MetaFunction = () => {
    return [
        { title: "HiddenFrame" },
        { name: "description", content: "Welcome to HiddenFrame!" },
    ];
};

export const loader: LoaderFunction = async ({ request }) => {
    const session = await getSession(request.headers.get("Cookie"));
    const username = session.get("username");

    const resp = await fetch(`${BASE_API_URL}/invites`, {
        headers: {
            "Authorization": session.get("token") || "",
            "Cookie": `token=${session.get("token")}`,
        }
    });

    if (!username) return redirect("/login");

    if (resp.ok) return { username, invites: await resp.json() };

    return { username };
};

export const action: ActionFunction = async ({ request }) => {
    const session = await getSession(request.headers.get("Cookie"));
    const token = session.get("token");
    const formData = await request.formData();
    const actionType = formData.get("actionType");

    if (actionType === "changePassword") {
        const newPassword = formData.get("password") as string;
        const confirmPassword = formData.get("confirm-password") as string;

        if (newPassword !== confirmPassword) {
            return { success: false, message: "Passwords do not match" };
        }

        const resp = await fetch(`${BASE_API_URL}/user`, {
            method: "PATCH",
            body: JSON.stringify({ password: newPassword }),
            headers: {
                "Content-Type": "application/json",
                "Authorization": token || "",
                "Cookie": `token=${token}`,
            }
        });

        if (resp.ok) {
            return { success: true, message: "Password changed successfully" };
        } else {
            const error = await resp.json();
            return { success: false, message: error.message };
        }
    } else if (actionType === "createInvite") {
        const resp = await fetch(`${BASE_API_URL}/invites/create`, {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Authorization": token || "",
                "Cookie": `token=${token}`,
            }
        });

        if (resp.ok) {
            const body = await resp.json();
            return { success: true, inviteId: body.inviteId };
        } else {
            const error = await resp.json();
            return { success: false, message: error.message };
        }
    }

    return { success: false, message: "Invalid action" };
};

type ActionData = {
    success: boolean;
    message?: string;
    inviteId?: string;
};

export default function Account() {
    const { invites } = useLoaderData<typeof loader>();
    const actionData = useActionData<ActionData>();
    const transition = useNavigation();
    const loading = transition.state === "submitting";

    const [inviteUrl, setInviteUrl] = useState<string>();

    useEffect(() => {
        if (actionData) {
            if (!actionData.success) {
                toast.error(actionData.message || "An error occurred");
            } else {
                if (actionData.inviteId) {
                    setInviteUrl(`${window.location.origin}/register?inviteId=${actionData.inviteId}`);
                }
                toast.success(actionData.message || "Action completed successfully");
            }
        }
    }, [actionData]);



    const copyKeyToClipboard = useMemo(() => async () => {
        if (inviteUrl) {
            await navigator.clipboard.writeText(inviteUrl);
            toast.success("Invite URL copied to clipboard");
        }
    }, [inviteUrl]);


    return (
        <div className="flex items-center justify-center h-full">
            <div className="flex flex-col items-center gap-8 h-full">
                <PageHeader />

                <h2 className="text-2xl font-black ">User Account</h2>

                <div className="flex flex-col gap-4 p-4">
                    <h3 className="text-xl">Change password</h3>
                    <Form method="post" className="flex flex-col gap-4 p-4">
                        <Input type="hidden" name="actionType" value="changePassword" />
                        <Input type="password" id="password" name="password" placeholder="Enter new password" />
                        <Input type="password" id="confirm-password" name="confirm-password" placeholder="Confirm new password" />
                        <Button loading={loading} type="submit">Change Password</Button>
                    </Form>
                </div>

                <div className="flex flex-col gap-4 p-4">
                    <h3 className="text-xl">Invites</h3>
                    {inviteUrl && <p>Invite created successfully. Invite URL: {inviteUrl}  <Button variant="ghost" size="sm" onClick={copyKeyToClipboard}><Copy /></Button></p>}
                    <p> You have {invites ? 5 - invites.length : 5} invite left</p>
                    <ul className="list-disc list-inside">
                        {invites && invites.map((invite: any) => <li key={invite.id}>{invite.id}</li>)}
                    </ul>

                    <Form method="post" className="flex flex-col gap-4 p-4">
                        <Input type="hidden" name="actionType" value="createInvite" />
                        <Button loading={loading} type="submit">Create New Invite</Button>
                    </Form>
                </div>
            </div>
        </div>
    );
}