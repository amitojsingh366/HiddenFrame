/* eslint-disable react/prop-types */

import { Loader2 } from "lucide-react";
import { ButtonProps, Button as ShadcnButton } from "./ui/button";

export const Button: React.FC<ButtonProps> = (props) => {
    const { children, loading, disabled, className } = props;
    return (
        <ShadcnButton
            disabled={disabled || loading}
            className={` ${className}`}
            {...props} >
            {loading && <Loader2 className="mr-2 h-4 w-4 animate-spin" />}
            {children}
        </ShadcnButton>
    );
}