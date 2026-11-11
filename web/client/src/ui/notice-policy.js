// ReceiveNotice packet types; welcome messages are explicitly system-chat per UI policy.
export function noticePolicy({message,type=1}){
 const branding=/^Powered by SSeMU$/i.test(message.trim())||message.trim()==='Phát triển T-NLabs - Mu Online',welcome=/^(?:Welcome(?: back| to)?\b|Chào mừng\b)/iu.test(message.trim());
 return {message:branding?'Phát triển T-NLabs - Mu Online':message,type:welcome?1:type,central:branding||!welcome&&(type===0||type===2||(type>=10&&type<=15))};
}
