import type { MetaFunction, ActionFunction, LoaderFunction } from "@remix-run/node";
import { json } from "@remix-run/node";
import { useLoaderData, Form, useActionData, useNavigation, redirect } from "@remix-run/react";
import { Button, PageHeader, Input } from "../components";
import { useEffect } from "react";
import { BASE_API_URL } from "../lib/consts";
import { toast } from "sonner";
import { getSession } from "../session";
import { InfoIcon } from "lucide-react";
import { Tooltip, TooltipContent, TooltipProvider, TooltipTrigger } from "../components/ui/tooltip";


export const loader: LoaderFunction = async ({ request, params }) => {
    const session = await getSession(request.headers.get("Cookie"));
    const username = session.get("username");

    const { id } = params;
    const imgUrl = `${BASE_API_URL}/static/${id}`;

    if (!username) redirect(imgUrl)

    return { imgUrl, username };
};

export const meta: MetaFunction<
    typeof loader,
    { "routes/u.$id": typeof loader }
> = ({ data }) => {
    return [
        { title: "HiddenFrame" },
        { property: "og:image", content: data.imgUrl },
        { name: "description", content: "View Image" },
    ];
};

export const action: ActionFunction = async ({ request, params }) => {
    const session = await getSession(request.headers.get("Cookie"));
    const token = session.get("token");
    const { id } = params;
    const formData = await request.formData();
    const key = formData.get("key") as string;

    const resp = await fetch(`${BASE_API_URL}/decode/${id}`, {
        method: "POST",
        body: JSON.stringify({ key }),
        headers: {
            "Content-Type": "application/json",
            "Authorization": token || "",
            "Cookie": `token=${token}`,
        }
    });

    if (resp.ok) {
        const body = await resp.json();
        if (body.success) {
            return json({ success: true, message: body.message }, { status: 200 });
        } else {
            return json({ success: false, message: body.message }, { status: 400 });
        }
    } else {
        const error = await resp.json();
        return json({ success: false, message: error.message }, { status: 500 });
    }
};

type ActionData = {
    success: boolean;
    message: string;
};

export default function ImagePage() {
    const { imgUrl, username } = useLoaderData<typeof loader>();
    const transition = useNavigation();
    const loading = transition.state === "submitting";
    const action = useActionData<ActionData>();

    useEffect(() => {
        if (action) {
            if (!action.success) {
                toast.error(action.message || "An error occurred");
            } else {
                toast.success("Decoded successfully");
            }
        }
    }, [action]);


    return (
        <div className="flex items-center justify-center h-full">
            <div className="flex flex-col items-center gap-16 h-full">
                <PageHeader />

                {imgUrl && (
                    <div className="flex flex-col items-center gap-4 p-4">
                        <div className="flex-none h-1/2">
                            <a className="flex items-center justify-center h-full" href={imgUrl} target="_blank" rel="noreferrer">
                                <img className="max-h-full object-contain rounded-lg" src={imgUrl} alt="Img loaded from backend" />
                            </a>
                        </div>
                        {action?.success &&
                            <div className="flex flex-col gap-6">
                                <h3 className="text-xl font-bold">Decoded Message</h3>
                                <p>{action.message}</p>
                            </div>
                        }
                        {username && (
                            <Form method="post" className="flex flex-col gap-4 p-4">
                                <div className="flex flex-row gap-2 items-center">
                                    <Input type="text" id="key" name="key" placeholder="Enter key..." />

                                    <TooltipProvider>
                                        <Tooltip>
                                            <TooltipTrigger type="button"><InfoIcon size={24} /></TooltipTrigger>
                                            <TooltipContent>
                                                <p>The image may not actually have a message embedded.</p>
                                            </TooltipContent>
                                        </Tooltip>
                                    </TooltipProvider>

                                </div>
                                <Button loading={loading} type="submit">Decode</Button>
                            </Form>
                        )}
                    </div>
                )}
            </div>
        </div>
    );
}