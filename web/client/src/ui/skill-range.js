// Server range/radius wins; PC client range fills only the skills where server data has no range.
export function castRange(meta){return Math.max(1,meta?.range||meta?.radius||meta?.clientRange||3);}
